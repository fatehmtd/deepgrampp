#include "listen-flux.hpp"

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
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace boost;
using namespace boost::beast;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using Websocket = websocket::stream<ssl::stream<tcp::socket>>;

namespace deepgram {
    namespace listen {
        namespace flux {
            class ListenFluxClientImpl {
                private:
                boost::asio::io_context _ioContext;
                boost::beast::net::ssl::context _sslContext;
                Websocket _webSocket;
                std::string _host;
                std::string _apiKey;
                std::string _port;

                std::thread _dataReceptionThread;
                std::atomic<bool> _keepReceiving;
                std::atomic<bool> _connected;

                public:
                ListenFluxClientImpl(const std::string& host,
                    const std::string& apiKey,
                    const std::string& port)
                    : _host(host), _apiKey(apiKey), _port(port),
                    _ioContext(), _sslContext(boost::beast::net::ssl::context::tlsv12_client),
                    _webSocket(_ioContext, _sslContext)
                {
                    _sslContext.set_default_verify_paths();
                    _sslContext.set_verify_mode(boost::beast::net::ssl::verify_peer);
                }

                ~ListenFluxClientImpl() {
                    close();
                }

                bool connect(const FluxQueryParams& params) {

                    if (_connected && _webSocket.is_open())
                    {
                        spdlog::warn("Already connected to Deepgram.");
                        return true;
                    }
                    try
                    {
                        std::string target = params.toQueryString();

                        spdlog::debug("target: {}", target);

                        tcp::resolver resolver{ _ioContext };
                        auto const results = resolver.resolve(_host, _port);

                        spdlog::debug("Connecting to {}:{} ...", _host, _port);
                        net::connect(_webSocket.next_layer().lowest_layer(), results.begin(), results.end());

                        spdlog::debug("Performing SSL handshake...");
                        _webSocket.next_layer().set_verify_callback(ssl::host_name_verification(_host));
                        _webSocket.next_layer().handshake(ssl::stream_base::client);

                        spdlog::debug("Performing WebSocket handshake...");
                        _webSocket.set_option(websocket::stream_base::decorator(
                            [this](websocket::request_type& req)
                            {
                                req.set(http::field::authorization, "Token " + _apiKey);
                                req.set(http::field::user_agent, "DeepgramCppClient/1.0");
                            }));

                        _webSocket.handshake(_host, target);
                        spdlog::debug("WebSocket connected successfully!");
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

                void startReceiving(const std::function<void(const std::string&)>& callBack) {
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
                                        spdlog::warn("WebSocket closed by server.");
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
                                catch (const std::exception& e)
                                {
                                    spdlog::error("Exception in receive loop: {}", e.what());
                                    break;
                                }
                            }

                            spdlog::debug("Receive loop ended."); });
                }

                void stopReceiving() {
                    if (!_keepReceiving) {
                        spdlog::warn("Receiving already stopped.");
                        return;
                    }
                    _keepReceiving = false;
                }

                void close() {
                    if (!_connected || !_webSocket.is_open()) {
                        spdlog::warn("WebSocket is not connected.");
                        return;
                    }
                    spdlog::debug("Closing connection...");
                    stopReceiving();

                    sendCloseStream();

                    // Wait a moment for final messages
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                    try
                    {
                        beast::error_code ec;
                        _webSocket.close(websocket::close_code::normal, ec);
                        if (websocket::error::closed == ec) {
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

                    _connected = false;
                    spdlog::debug("Connection closed.");
                }

                bool streamAudio(const std::vector<uint8_t>& audioData, size_t chunkSize = 4000)
                {
                    if (!_connected || !_webSocket.is_open())
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

                bool sendAudioChunk(const uint8_t* data, size_t size)
                {
                    if (!_connected || !_webSocket.is_open())
                    {
                        spdlog::error("Not connected to Deepgram.");
                        return false;
                    }
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
                    catch (const std::exception& e)
                    {
                        spdlog::error("Send chunk error: {}", e.what());
                        return false;
                    }
                }

                void sendCloseStream()
                {
                    if (!_connected || !_webSocket.is_open()) {
                        spdlog::warn("WebSocket is not connected.");
                        return;
                    }
                    try
                    {
                        // Send as text message
                        _webSocket.text(true);
                        beast::error_code ec;

                        // prepare the payload
                        nlohmann::json jsonPayload;
                        jsonPayload["type"] = "CloseStream";

                        _webSocket.write(net::buffer(jsonPayload.dump()), ec);

                        if (ec)
                        {
                            spdlog::error("Error sending close stream: {}", ec.message());
                        }
                        else
                        {
                            spdlog::debug("Sent close stream message.");
                        }
                    }
                    catch (const std::exception& e)
                    {
                        spdlog::error("Error in sendCloseStream: {}", e.what());
                    }
                }
            };
        }
    }
}