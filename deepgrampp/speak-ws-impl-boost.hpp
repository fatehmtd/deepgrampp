#pragma once
#include "include/deepgrampp/speak-ws.hpp"
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
            Websocket _webSocket;
            std::string _host;
            std::string _apiKey;
            std::string _port;

            std::thread _dataReceptionThread;
            std::thread _keepaliveThread;
            std::atomic<bool> _keepReceiving;
            std::atomic<bool> connected_;

        public:
            SpeakWebsocketClientImpl(const std::string &host,
                                      const std::string &apiKey,
                                      const std::string &port)
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
                return connected_;
            }

            bool connect(const LiveSpeakConfig &config)
            {
                try
                {
                    std::string target = config.toQueryString();

                    spdlog::info("target: {}", target);

                    tcp::resolver resolver{_ioContext};
                    auto const results = resolver.resolve(_host, _port);

                    spdlog::info("Connecting to {}:{} ...", _host, _port);
                    net::connect(_webSocket.next_layer().lowest_layer(), results.begin(), results.end());

                    spdlog::info("Performing SSL handshake...");
                    _webSocket.next_layer().set_verify_callback(ssl::host_name_verification(_host));
                    _webSocket.next_layer().handshake(ssl::stream_base::client);

                    spdlog::info("Performing WebSocket handshake...");
                    _webSocket.set_option(websocket::stream_base::decorator(
                        [this](websocket::request_type &req)
                        {
                            req.set(http::field::authorization, "Token " + _apiKey);
                            req.set(http::field::user_agent, "DeepgramCppClient/1.0");
                        }));

                    _webSocket.handshake(_host, target);
                    spdlog::info("WebSocket connected successfully!");
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

            bool startReceiving(
                std::function<void(const char*, int)> audioCallback,
                std::function<void(const std::string&)> textCallback
            )
            {
                _dataReceptionThread = std::thread([this, audioCallback, textCallback]()
                                                   {
                spdlog::info("Starting receive loop");
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

                        if(_webSocket.got_binary()) {
                            // binary data represents speech bytes
                            audioCallback(reinterpret_cast<const char*>(buffer.data().data()), bytesRead);
                        } else {
                            // text data represents json responses
                            std::string message = beast::buffers_to_string(buffer.data());
                            spdlog::info("Received text message: {}", message);
                            textCallback(message);
                        }
                        buffer.consume(bytesRead);
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Exception in receive loop: {}", e.what());
                        break;
                    }
                }

                spdlog::info("Receive loop ended."); });
            }

            bool sendPayload(const std::string &payload)
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
                catch (const std::exception &e)
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
                spdlog::info("Connection closed.");
            }
        };
    }
}