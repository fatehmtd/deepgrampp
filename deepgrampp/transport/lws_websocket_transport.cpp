#include <deepgrampp/transport/lws_websocket_transport.hpp>

#include <libwebsockets.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <map>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace deepgram
{
    namespace transport
    {

        // ---------------------------------------------------------------------------
        // Impl
        // ---------------------------------------------------------------------------

        struct LwsWebSocketTransportImpl
        {
            std::string _address;
            int _port{0};
            std::string _path;
            bool _tls{true};
            std::map<std::string, std::string> _headers;
            std::string _caFilePath;

            lws_context *_ctx{nullptr};
            lws *_wsi{nullptr};

            std::thread _serviceThread;
            std::atomic<bool> _stopping{false};

            std::mutex _connectMutex;
            std::condition_variable _connectCv;
            bool _connectDone{false};
            bool _connectFailed{false};
            std::string _connectError;

            struct OutboundMsg
            {
                bool is_binary{false};
                std::vector<unsigned char> data;
            };
            std::mutex _queueMutex;
            std::queue<OutboundMsg> _sendQueue;

            std::atomic<bool> _closing{false};

            std::vector<uint8_t> _fragBuf;
            bool _fragIsBinary{false};

            mutable std::mutex _callbackMutex;
            IWebSocketTransport::OpenHandler _onOpen;
            IWebSocketTransport::TextMessageHandler _onText;
            IWebSocketTransport::BinaryMessageHandler _onBinary;
            IWebSocketTransport::ErrorHandler _onError;
            IWebSocketTransport::CloseHandler _onClose;
            std::atomic<bool> _isOpen{false};
        };

        namespace
        {

            void emitError(LwsWebSocketTransportImpl *impl, const std::string &error)
            {
                IWebSocketTransport::ErrorHandler cb;
                {
                    std::lock_guard<std::mutex> g(impl->_callbackMutex);
                    cb = impl->_onError;
                }
                if (cb)
                {
                    cb(error);
                }
            }

            void resetConnectionState(LwsWebSocketTransportImpl *impl)
            {
                impl->_stopping.store(false);
                impl->_closing.store(false);
                impl->_isOpen.store(false);
                impl->_connectDone = false;
                impl->_connectFailed = false;
                impl->_connectError.clear();
                impl->_fragBuf.clear();
                impl->_fragIsBinary = false;
                impl->_wsi = nullptr;

                std::queue<LwsWebSocketTransportImpl::OutboundMsg> emptyQueue;
                std::lock_guard<std::mutex> lk(impl->_queueMutex);
                impl->_sendQueue.swap(emptyQueue);
            }

            void cleanupConnection(LwsWebSocketTransportImpl *impl)
            {
                impl->_stopping.store(true);
                if (impl->_ctx)
                {
                    lws_cancel_service(impl->_ctx);
                }
                if (impl->_serviceThread.joinable())
                {
                    impl->_serviceThread.join();
                }
                if (impl->_ctx)
                {
                    lws_context_destroy(impl->_ctx);
                    impl->_ctx = nullptr;
                }
                impl->_wsi = nullptr;
            }

        } // namespace

        // ---------------------------------------------------------------------------
        // URL parser
        // ---------------------------------------------------------------------------

        namespace
        {

            struct ParsedWsUrl
            {
                std::string address;
                int port{443};
                std::string path;
                bool tls{true};
            };

            ParsedWsUrl parseWsUrl(const std::string &url)
            {
                ParsedWsUrl out;
                std::string rest;

                if (url.rfind("wss://", 0) == 0)
                {
                    out.tls = true;
                    out.port = 443;
                    rest = url.substr(6);
                }
                else if (url.rfind("ws://", 0) == 0)
                {
                    out.tls = false;
                    out.port = 80;
                    rest = url.substr(5);
                }
                else
                {
                    throw std::runtime_error("[deepgrampp] Invalid WebSocket URL: " + url);
                }

                const auto slash = rest.find('/');
                const auto hostPort = (slash != std::string::npos) ? rest.substr(0, slash) : rest;
                out.path = (slash != std::string::npos) ? rest.substr(slash) : "/";

                const auto colon = hostPort.find(':');
                if (colon != std::string::npos)
                {
                    out.address = hostPort.substr(0, colon);
                    out.port = std::stoi(hostPort.substr(colon + 1));
                }
                else
                {
                    out.address = hostPort;
                }
                return out;
            }

        } // namespace

        // ---------------------------------------------------------------------------
        // Forward declarations
        // ---------------------------------------------------------------------------

        static int lwsCallback(lws *wsi, lws_callback_reasons reason,
                               void *user, void *in, size_t len);

        static const lws_protocols kProtocols[] = {
            {"deepgrampp-ws", lwsCallback, 0, 65536, 0, nullptr, 0},
            LWS_PROTOCOL_LIST_TERM};

        // ---------------------------------------------------------------------------
        // LWS callback
        // ---------------------------------------------------------------------------

        static int lwsCallback(lws *wsi, lws_callback_reasons reason,
                               void * /*user*/, void *in, size_t len)
        {
            auto *impl = static_cast<LwsWebSocketTransportImpl *>(
                lws_context_user(lws_get_context(wsi)));
            if (!impl)
                return 0;

            switch (reason)
            {

            case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            {
                auto **p = reinterpret_cast<unsigned char **>(in);
                auto *end = *p + len;
                for (const auto &[key, value] : impl->_headers)
                {
                    if (lws_add_http_header_by_name(
                            wsi,
                            reinterpret_cast<const unsigned char *>(key.c_str()),
                            reinterpret_cast<const unsigned char *>(value.c_str()),
                            static_cast<int>(value.size()),
                            p, end) != 0)
                    {
                        return -1;
                    }
                }
                break;
            }

            case LWS_CALLBACK_CLIENT_ESTABLISHED:
            {
                {
                    std::lock_guard<std::mutex> lk(impl->_connectMutex);
                    impl->_connectDone = true;
                    impl->_connectFailed = false;
                }
                impl->_connectCv.notify_one();
                break;
            }

            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            {
                const std::string err = (in && len > 0)
                    ? std::string(static_cast<const char *>(in), len)
                    : "connection error";
                {
                    std::lock_guard<std::mutex> lk(impl->_connectMutex);
                    impl->_connectDone = true;
                    impl->_connectFailed = true;
                    impl->_connectError = err;
                }
                impl->_connectCv.notify_one();
                impl->_wsi = nullptr;
                break;
            }

            case LWS_CALLBACK_CLIENT_RECEIVE:
            {
                const bool is_bin = (lws_frame_is_binary(wsi) != 0);
                const auto *data = static_cast<const uint8_t *>(in);

                if (impl->_fragBuf.empty())
                {
                    impl->_fragIsBinary = is_bin;
                }
                impl->_fragBuf.insert(impl->_fragBuf.end(), data, data + len);

                if (lws_is_final_fragment(wsi))
                {
                    if (impl->_fragIsBinary)
                    {
                        IWebSocketTransport::BinaryMessageHandler cb;
                        {
                            std::lock_guard<std::mutex> g(impl->_callbackMutex);
                            cb = impl->_onBinary;
                        }
                        if (cb)
                            cb(impl->_fragBuf);
                    }
                    else
                    {
                        IWebSocketTransport::TextMessageHandler cb;
                        {
                            std::lock_guard<std::mutex> g(impl->_callbackMutex);
                            cb = impl->_onText;
                        }
                        if (cb)
                        {
                            try
                            {
                                cb(std::string(impl->_fragBuf.begin(), impl->_fragBuf.end()));
                            }
                            catch (const std::exception &ex)
                            {
                                emitError(impl, std::string("[deepgrampp] text message callback threw: ") + ex.what());
                            }
                            catch (...)
                            {
                                emitError(impl, "[deepgrampp] text message callback threw unknown exception");
                            }
                        }
                    }
                    impl->_fragBuf.clear();
                }
                break;
            }

            case LWS_CALLBACK_CLIENT_WRITEABLE:
            {
                if (impl->_closing.load())
                {
                    lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
                    return -1;
                }

                std::lock_guard<std::mutex> lk(impl->_queueMutex);
                if (!impl->_sendQueue.empty())
                {
                    auto &msg = impl->_sendQueue.front();
                    const size_t paylen = msg.data.size() - LWS_PRE;
                    const int written = lws_write(wsi,
                                                  msg.data.data() + LWS_PRE,
                                                  paylen,
                                                  msg.is_binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT);
                    if (written < 0 || static_cast<size_t>(written) != paylen)
                    {
                        impl->_closing.store(true);
                        impl->_isOpen.store(false);
                        emitError(impl, "[deepgrampp] lws_write failed");
                        return -1;
                    }
                    impl->_sendQueue.pop();
                    if (!impl->_sendQueue.empty())
                    {
                        lws_callback_on_writable(wsi);
                    }
                }
                break;
            }

            case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            {
                if (impl->_wsi)
                {
                    std::lock_guard<std::mutex> lk(impl->_queueMutex);
                    if (!impl->_sendQueue.empty() || impl->_closing.load())
                    {
                        lws_callback_on_writable(impl->_wsi);
                    }
                }
                break;
            }

            case LWS_CALLBACK_CLIENT_CLOSED:
            case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
            {
                impl->_wsi = nullptr;
                impl->_stopping.store(true);

                if (impl->_isOpen.exchange(false))
                {
                    IWebSocketTransport::CloseHandler cb;
                    {
                        std::lock_guard<std::mutex> g(impl->_callbackMutex);
                        cb = impl->_onClose;
                    }
                    if (cb)
                        cb();
                }
                break;
            }

            default:
                break;
            }
            return 0;
        }

        // ---------------------------------------------------------------------------
        // Construction / destruction
        // ---------------------------------------------------------------------------

        LwsWebSocketTransport::LwsWebSocketTransport(std::string caFilePath)
            : _impl(std::make_unique<LwsWebSocketTransportImpl>())
        {
            _impl->_caFilePath = std::move(caFilePath);
        }

        LwsWebSocketTransport::~LwsWebSocketTransport()
        {
            if (_impl->_isOpen.load())
            {
                _impl->_closing.store(true);
                if (_impl->_ctx)
                    lws_cancel_service(_impl->_ctx);
            }
            _impl->_stopping.store(true);
            if (_impl->_ctx)
                lws_cancel_service(_impl->_ctx);

            if (_impl->_serviceThread.joinable())
            {
                _impl->_serviceThread.join();
            }
            if (_impl->_ctx)
            {
                lws_context_destroy(_impl->_ctx);
                _impl->_ctx = nullptr;
            }
        }

        // ---------------------------------------------------------------------------
        // Handler setters
        // ---------------------------------------------------------------------------

        void LwsWebSocketTransport::setOnOpen(OpenHandler h)
        {
            std::lock_guard<std::mutex> g(_impl->_callbackMutex);
            _impl->_onOpen = std::move(h);
        }
        void LwsWebSocketTransport::setOnTextMessage(TextMessageHandler h)
        {
            std::lock_guard<std::mutex> g(_impl->_callbackMutex);
            _impl->_onText = std::move(h);
        }
        void LwsWebSocketTransport::setOnBinaryMessage(BinaryMessageHandler h)
        {
            std::lock_guard<std::mutex> g(_impl->_callbackMutex);
            _impl->_onBinary = std::move(h);
        }
        void LwsWebSocketTransport::setOnError(ErrorHandler h)
        {
            std::lock_guard<std::mutex> g(_impl->_callbackMutex);
            _impl->_onError = std::move(h);
        }
        void LwsWebSocketTransport::setOnClose(CloseHandler h)
        {
            std::lock_guard<std::mutex> g(_impl->_callbackMutex);
            _impl->_onClose = std::move(h);
        }
        bool LwsWebSocketTransport::isOpen() const { return _impl->_isOpen.load(); }

        // ---------------------------------------------------------------------------
        // connect()
        // ---------------------------------------------------------------------------

        void LwsWebSocketTransport::connect(const WebSocketConnectOptions &options)
        {
            lws_set_log_level(LLL_ERR | LLL_WARN, nullptr);

            cleanupConnection(_impl.get());
            resetConnectionState(_impl.get());

            const ParsedWsUrl parsed = parseWsUrl(options.url);
            _impl->_address = parsed.address;
            _impl->_port = parsed.port;
            _impl->_path = parsed.path;
            _impl->_tls = parsed.tls;
            _impl->_headers = options.headers;

            lws_context_creation_info ctx_info{};
            ctx_info.port = CONTEXT_PORT_NO_LISTEN;
            ctx_info.protocols = kProtocols;
            ctx_info.user = _impl.get();
            ctx_info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

            // mbedTLS (our TLS backend everywhere except Windows) ships with no built-in
            // trust anchors, so without an explicit CA file every handshake fails with
            // "CA is not trusted" -- see the caFilePath constructor argument.
            if (!_impl->_caFilePath.empty())
            {
                ctx_info.client_ssl_ca_filepath = _impl->_caFilePath.c_str();
            }

            _impl->_ctx = lws_create_context(&ctx_info);
            if (!_impl->_ctx)
            {
                throw std::runtime_error("[deepgrampp] lws_create_context failed");
            }

            lws_client_connect_info ccinfo{};
            ccinfo.context = _impl->_ctx;
            ccinfo.address = _impl->_address.c_str();
            ccinfo.port = _impl->_port;
            ccinfo.path = _impl->_path.c_str();
            ccinfo.host = _impl->_address.c_str();
            ccinfo.origin = _impl->_address.c_str();
            ccinfo.protocol = kProtocols[0].name;
            ccinfo.ssl_connection = _impl->_tls ? LCCSCF_USE_SSL : 0;

            _impl->_wsi = lws_client_connect_via_info(&ccinfo);
            if (!_impl->_wsi)
            {
                lws_context_destroy(_impl->_ctx);
                _impl->_ctx = nullptr;
                throw std::runtime_error("[deepgrampp] lws_client_connect_via_info failed");
            }

            _impl->_serviceThread = std::thread([this]
                                                {
                while (!_impl->_stopping.load()) {
                    lws_service(_impl->_ctx, 50);
                } });

            {
                std::unique_lock<std::mutex> lk(_impl->_connectMutex);
                constexpr auto connectTimeout = std::chrono::seconds(15);
                const bool connected = _impl->_connectCv.wait_for(
                    lk,
                    connectTimeout,
                    [this]
                    { return _impl->_connectDone; });
                if (!connected)
                {
                    _impl->_connectFailed = true;
                    _impl->_connectError = "connection timed out";
                }
            }

            if (_impl->_connectFailed)
            {
                cleanupConnection(_impl.get());
                throw std::runtime_error("[deepgrampp] WebSocket connect failed: " + _impl->_connectError);
            }

            _impl->_isOpen.store(true);

            IWebSocketTransport::OpenHandler cb;
            {
                std::lock_guard<std::mutex> g(_impl->_callbackMutex);
                cb = _impl->_onOpen;
            }
            if (cb)
                cb();
        }

        // ---------------------------------------------------------------------------
        // sendText() / sendBinary() / close()
        // ---------------------------------------------------------------------------

        void LwsWebSocketTransport::sendText(const std::string &message)
        {
            if (!_impl->_isOpen.load())
            {
                throw std::runtime_error("[deepgrampp] WebSocket is not open");
            }

            LwsWebSocketTransportImpl::OutboundMsg msg;
            msg.is_binary = false;
            msg.data.resize(LWS_PRE + message.size());
            std::memcpy(msg.data.data() + LWS_PRE, message.data(), message.size());

            {
                std::lock_guard<std::mutex> lk(_impl->_queueMutex);
                _impl->_sendQueue.push(std::move(msg));
            }
            lws_cancel_service(_impl->_ctx);
        }

        void LwsWebSocketTransport::sendBinary(const std::vector<std::uint8_t> &payload)
        {
            if (!_impl->_isOpen.load())
            {
                throw std::runtime_error("[deepgrampp] WebSocket is not open");
            }

            LwsWebSocketTransportImpl::OutboundMsg msg;
            msg.is_binary = true;
            msg.data.resize(LWS_PRE + payload.size());
            if (!payload.empty())
            {
                std::memcpy(msg.data.data() + LWS_PRE, payload.data(), payload.size());
            }

            {
                std::lock_guard<std::mutex> lk(_impl->_queueMutex);
                _impl->_sendQueue.push(std::move(msg));
            }
            lws_cancel_service(_impl->_ctx);
        }

        void LwsWebSocketTransport::close()
        {
            if (!_impl->_isOpen.exchange(false))
                return;

            _impl->_closing.store(true);
            lws_cancel_service(_impl->_ctx);
        }

    } // namespace transport
} // namespace deepgram
