#include "include/deepgrampp/listen-ws.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/beast/http.hpp>
#include <functional>
#include <spdlog/spdlog.h>

using namespace boost;
using namespace boost::beast;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using Websocket = websocket::stream<ssl::stream<tcp::socket>>;

namespace deepgram
{
    namespace listen
    {
        class ListenWebsocketClientImpl
        {
        private:
            boost::asio::io_context _ioContext;
            boost::beast::net::ssl::context _sslContext;
            Websocket _webSocket;
            std::string _host;
            std::string _apiKey;
            std::string _port;

            std::thread _dataReceptionThread;
            std::thread _keepaliveThread;
            std::atomic<bool> _keepReceiving;
            std::atomic<bool> connected_;

        public:
            ListenWebsocketClientImpl(const std::string &host,
                                      const std::string &apiKey,
                                      const std::string &port)
                : _host(host), _apiKey(apiKey), _port(port),
                  _ioContext(), _sslContext(boost::beast::net::ssl::context::tlsv12_client),
                  _webSocket(_ioContext, _sslContext)
            {
                _sslContext.set_default_verify_paths();
                _sslContext.set_verify_mode(boost::beast::net::ssl::verify_peer);
            }

            ~ListenWebsocketClientImpl()
            {
                close();
            }

            bool connect(const LiveTranscriptionOptions &options)
            {
                try
                {
                    std::string target = options.toQueryString();

                    spdlog::debug("target: {}", target);

                    tcp::resolver resolver{_ioContext};
                    auto const results = resolver.resolve(_host, _port);

                    spdlog::debug("Connecting to {}:{} ...", _host, _port);
                    net::connect(_webSocket.next_layer().lowest_layer(), results.begin(), results.end());

                    spdlog::debug("Performing SSL handshake...");
                    _webSocket.next_layer().set_verify_callback(ssl::host_name_verification(_host));
                    _webSocket.next_layer().handshake(ssl::stream_base::client);

                    spdlog::debug("Performing WebSocket handshake...");
                    _webSocket.set_option(websocket::stream_base::decorator(
                        [this](websocket::request_type &req)
                        {
                            req.set(http::field::authorization, "Token " + _apiKey);
                            req.set(http::field::user_agent, "DeepgramCppClient/1.0");
                        }));

                    _webSocket.handshake(_host, target);
                    spdlog::debug("WebSocket connected successfully!");
                    connected_ = true;
                    _keepReceiving = true;
                    return true;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Connection error: {}", e.what());
                    return false;
                }
            }

            void startReceiving(const std::function<void(const std::string &)> &callBack)
            {
                _dataReceptionThread = std::thread([this, callBack]()
                                                   {
                spdlog::debug("Starting receive loop");
                beast::flat_buffer buffer;
                beast::error_code ec;
                while (_keepReceiving && _webSocket.is_open())
                {
                    try
                    {
                        size_t bytesRead = _webSocket.read(buffer, ec);

                        if (ec == websocket::error::closed)
                        {
                            spdlog::error("WebSocket closed by server.");
                            break;
                        }

                        if (ec)
                        {
                            spdlog::error("Read error: {}", ec.message());
                            break;
                        }
                        std::string message = beast::buffers_to_string(buffer.data());
                        callBack(message);
                        buffer.consume(bytesRead);
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Exception in receive loop: {}", e.what());
                        break;
                    }
                }

                spdlog::debug("Receive loop ended."); });
            }

            void startKeepalive()
            {
                _keepaliveThread = std::thread([this]()
                                               {
            spdlog::debug("Starting keepalive thread...");
            
            while (_keepReceiving && _webSocket.is_open())
            {
                std::this_thread::sleep_for(std::chrono::seconds(5)); // Send every 5 seconds
                
                if (!_keepReceiving || !_webSocket.is_open()) break;
                
                try
                {
                    beast::error_code ec;
                    _webSocket.write(net::buffer(std::string(deepgram::listen::control::KEEPALIVE_MESSAGE)), ec);
                    
                    if (ec)
                    {
                        spdlog::error("Keepalive send error: {}", ec.message());
                        break;
                    }
                    
                    spdlog::debug("Sent keepalive message");
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Keepalive error: {}", e.what());
                    break;
                }
            }
            
            spdlog::debug("Keepalive thread ended"); });
            }

            bool streamAudioFile(const std::vector<uint8_t> &audioData, size_t chunkSize = 4000)
            {
                if (!connected_ || !_webSocket.is_open())
                {
                    spdlog::error("Not connected to Deepgram.");
                    return false;
                }
                spdlog::debug("Streaming {} bytes of audio...", audioData.size());

                size_t offset = 0;
                int chunkNumber = 0;
                int numChunks = audioData.size() / chunkSize;

                while (offset < audioData.size() && _webSocket.is_open())
                {
                    size_t currentChunkSize = std::min(chunkSize, audioData.size() - offset);

                    spdlog::debug("Sending chunk: {}/{} -> {} bytes...", ++chunkNumber, numChunks, currentChunkSize);

                    if (!sendAudioChunk(audioData.data() + offset, currentChunkSize))
                    {
                        spdlog::error("Failed to send audio chunk");
                        return false;
                    }

                    offset += currentChunkSize;
                }
                spdlog::info("Finished streaming audio data.");
                return true;
            }

            bool sendFinalizeMessage()
            {
                if (!_webSocket.is_open())
                {
                    spdlog::error("can't send finalize message, websocket not open");
                    return false;
                }
                try
                {
                    beast::error_code ec;
                    // prepare to send text
                    _webSocket.binary(false);
                    _webSocket.write(net::buffer(std::string(control::FINALIZE_MESSAGE)), ec);
                    if (ec)
                    {
                        spdlog::error("Write error: {}", ec.message());
                        return false;
                    }
                }
                catch (const std::exception &e)
                {
                    spdlog::error("sendFinalizeMessage error: {}", e.what());
                    return false;
                }
                return true;
            }

            bool sendAudioChunk(const uint8_t *data, size_t size)
            {
                try
                {
                    beast::error_code ec;
                    // Send as binary WebSocket message
                    _webSocket.binary(true);
                    int bytesOut = _webSocket.write(net::buffer(data, size), ec);
                    if (ec)
                    {
                        spdlog::error("Write error: {}", ec.message());
                        return false;
                    }
                    return true;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Send chunk error: {}", e.what());
                    return false;
                }
            }

            void sendCloseStream()
            {
                try
                {
                    // Send as text message
                    _webSocket.binary(false);
                    beast::error_code ec;
                    _webSocket.write(net::buffer(std::string(deepgram::listen::control::CLOSE_MESSAGE)), ec);

                    if (ec)
                    {
                        spdlog::error("Error sending close stream: {}", ec.message());
                    }
                    else
                    {
                        spdlog::debug("Sent close stream message.");
                    }
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Error in sendCloseStream: {}", e.what());
                }
            }

            void stopReceiving()
            {
                _keepReceiving = false;
            }

            void close()
            {
                spdlog::debug("Closing connection...");

                stopReceiving();

                if (connected_ && _webSocket.is_open())
                {
                    sendCloseStream();

                    // Wait a moment for final messages
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                    try
                    {
                        beast::error_code ec;
                        _webSocket.close(websocket::close_code::normal, ec);
                        if (ec && ec != websocket::error::closed)
                        {
                            spdlog::error("Close error: {}", ec.message());
                        }
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Exception during close: {}", e.what());
                    }
                }

                try
                {
                    if (_dataReceptionThread.joinable())
                    {
                        _dataReceptionThread.join();
                    }
                }
                catch (const std::exception &e)
                {
                }

                try
                {
                    if (_keepaliveThread.joinable())
                    {
                        _keepaliveThread.join();
                    }
                }
                catch (const std::exception &e)
                {
                }

                connected_ = false;
                spdlog::debug("Connection closed.");
            }
        };
    }
}