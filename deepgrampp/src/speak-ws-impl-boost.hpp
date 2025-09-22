#pragma once
#include "../include/deepgrampp/speak-ws.hpp"
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
    namespace speak
    {
        class SpeakWebsocketClientImpl
        {
            private:
            boost::asio::io_context _ioContext;
            boost::beast::net::ssl::context _sslContext;
            mutable Websocket _webSocket;
            std::string _host;
            std::string _apiKey;
            std::string _port;

            std::thread _dataReceptionThread;
            std::thread _keepaliveThread;
            std::thread _timeoutThread;
            std::atomic<bool> _keepReceiving;
            std::atomic<bool> _connected;
            std::atomic<bool> _receivingSpeech = false;
            std::atomic<uint64_t> _lastSpeechMessageTime = 0;
            const int _speechReceptionTimeoutMs = 1500;

            public:
            SpeakWebsocketClientImpl(const std::string& host,
                const std::string& apiKey,
                const std::string& port)
                : _host(host), _apiKey(apiKey), _port(port),
                _ioContext(), _sslContext(boost::beast::net::ssl::context::tlsv12_client),
                _webSocket(_ioContext, _sslContext)
            {
                _sslContext.set_default_verify_paths();
                _sslContext.set_verify_mode(boost::beast::net::ssl::verify_peer);
            }

            ~SpeakWebsocketClientImpl()
            {
                close();
            }

            bool isConnected() const
            {
                return _connected;
            }

            bool connect(const LiveSpeakConfig& config)
            {
                try
                {
                    std::string target = config.toQueryString();

                    spdlog::info("target: {}", target);

                    tcp::resolver resolver{ _ioContext };
                    auto const results = resolver.resolve(_host, _port);

                    spdlog::info("Connecting to {}:{} ...", _host, _port);
                    net::connect(_webSocket.next_layer().lowest_layer(), results.begin(), results.end());

                    spdlog::info("Performing SSL handshake...");
                    _webSocket.next_layer().set_verify_callback(ssl::host_name_verification(_host));
                    _webSocket.next_layer().handshake(ssl::stream_base::client);

                    spdlog::info("Performing WebSocket handshake...");
                    _webSocket.set_option(websocket::stream_base::decorator(
                        [this](websocket::request_type& req)
                        {
                            req.set(http::field::authorization, "Token " + _apiKey);
                            req.set(http::field::user_agent, "DeepgramCppClient/1.0");
                        }));

                    _webSocket.handshake(_host, target);
                    spdlog::info("WebSocket connected successfully!");
                    _connected = true;
                    _keepReceiving = true;
                    return true;
                }
                catch (const std::exception& e)
                {
                    spdlog::error("Connection error: {}", e.what());
                    return false;
                }
            }

            bool startReceiving(
                std::function<void(const char*, int)> audioCallback,
                std::function<void(const std::string&)> textCallback,
                std::function<void(const std::string&)> errorCallback = nullptr,
                std::function<void()> disconnectedCallback = nullptr,
                std::function<void()> speechStartedCallback = nullptr,
                std::function<void()> speechEndedCallback = nullptr
            )
            {
                _timeoutThread = std::thread([this,
                    speechEndedCallback] {
                        try {
                            while (_keepReceiving && _webSocket.is_open()) {
                                // Check for speech timeout
                                if (_receivingSpeech) {
                                    uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                                    spdlog::debug("Current time: {}, Last speech time: {}, delta: {}", currentTime, _lastSpeechMessageTime.load(), currentTime - _lastSpeechMessageTime.load());
                                    if (currentTime - _lastSpeechMessageTime > _speechReceptionTimeoutMs) {
                                        spdlog::warn("No speech data received for {} milliseconds, assuming end of speech.", _speechReceptionTimeoutMs);
                                        _receivingSpeech = false;
                                        if (speechEndedCallback) {
                                            speechEndedCallback();
                                        }
                                    }
                                }
                                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                            }
                        }
                        catch (const std::exception& e) {
                            spdlog::error("Exception in timeout thread: {}", e.what());
                        }
                    });
                _dataReceptionThread = std::thread([this,
                    audioCallback,
                    textCallback,
                    errorCallback,
                    disconnectedCallback,
                    speechEndedCallback,
                    speechStartedCallback]()
                    {
                        spdlog::info("Starting receive loop");
                        beast::flat_buffer buffer;
                        beast::error_code ec;
                        // Set a read timeout
                        _webSocket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
                        while (_keepReceiving && _webSocket.is_open())
                        {
                            try
                            {
                                size_t bytesRead = _webSocket.read(buffer, ec);

                                if (ec)
                                {
                                    if (ec == websocket::error::closed)
                                    {
                                        spdlog::warn("WebSocket closed by server.");
                                        if (disconnectedCallback) {
                                            disconnectedCallback();
                                        }
                                        break;
                                    }
                                    else {
                                        spdlog::error("Read error: {}", ec.message());
                                        if (errorCallback) {
                                            errorCallback(ec.message());
                                        }
                                        if (disconnectedCallback) {
                                            disconnectedCallback();
                                        }
                                        break;
                                    }
                                }

                                if (_webSocket.got_binary()) {
                                    if (!_receivingSpeech) {
                                        if (speechStartedCallback) {
                                            speechStartedCallback();
                                        }
                                        _receivingSpeech = true;
                                    }
                                    // binary data represents speech bytes
                                    audioCallback(reinterpret_cast<const char*>(buffer.data().data()), bytesRead);
                                    _lastSpeechMessageTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                                }
                                else {
                                    // text data represents json responses
                                    std::string message = beast::buffers_to_string(buffer.data());
                                    spdlog::info("Received text message: {}", message);
                                    textCallback(message);
                                }
                                buffer.consume(bytesRead);
                            }
                            catch (const std::exception& e)
                            {
                                spdlog::error("Exception in receive loop: {}", e.what());
                                break;
                            }
                        }

                        spdlog::info("Receive loop ended."); });
                return true;
            }

            bool sendPayload(const std::string& payload)
            {
                if (!_webSocket.is_open())
                {
                    spdlog::error("can't send payload, websocket not open");
                    return false;
                }
                try
                {
                    spdlog::info("Sending payload: {}", payload);
                    beast::error_code ec;
                    _webSocket.text(true);
                    _webSocket.write(net::buffer(payload), ec);
                    if (ec)
                    {
                        spdlog::error("Write error: {}", ec.message());
                        return false;
                    }
                }
                catch (const std::exception& e)
                {
                    spdlog::error("sendPayload error: {}", e.what());
                    return false;
                }
                return true;
            }

            bool sendFlushMessage()
            {
                return sendPayload(std::string(control::FLUSH));
            }

            bool sendCloseStream()
            {
                return sendPayload(std::string(control::CLOSE));
            }

            void stopReceiving()
            {
                _keepReceiving = false;
            }

            void close()
            {
                spdlog::info("Closing connection...");

                stopReceiving();

                if (_connected && _webSocket.is_open())
                {
                    sendCloseStream();

                    // Wait a moment for final messages
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                    try
                    {
                        beast::error_code ec;
                        _webSocket.close(websocket::close_code::normal, ec);
                        if (ec == websocket::error::closed) {
                            spdlog::warn("WebSocket already closed.");
                        }
                        else if (ec && ec != websocket::error::closed)
                        {
                            spdlog::error("Close error: {}", ec.message());
                        }
                    }
                    catch (const std::exception& e)
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
                catch (const std::exception& e)
                {
                }

                try
                {
                    if (_keepaliveThread.joinable())
                    {
                        _keepaliveThread.join();
                    }
                }
                catch (const std::exception& e)
                {
                }

                try
                {
                    if (_timeoutThread.joinable())
                    {
                        _timeoutThread.join();
                    }
                }
                catch (const std::exception& e)
                {
                }

                _connected = false;
                spdlog::info("Connection closed.");
            }
        };
    }
}