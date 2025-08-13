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

using namespace deepgram::listen;

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

                    std::cout << "target: " << target << std::endl;

                    tcp::resolver resolver{_ioContext};
                    auto const results = resolver.resolve(_host, _port);

                    std::cout << "Connecting to " << _host << ":" << _port << "..." << std::endl;
                    net::connect(_webSocket.next_layer().lowest_layer(), results.begin(), results.end());

                    std::cout << "Performing SSL handshake..." << std::endl;
                    _webSocket.next_layer().set_verify_callback(ssl::host_name_verification(_host));
                    _webSocket.next_layer().handshake(ssl::stream_base::client);

                    std::cout << "Performing WebSocket handshake..." << std::endl;
                    _webSocket.set_option(websocket::stream_base::decorator(
                        [this](websocket::request_type &req)
                        {
                            req.set(http::field::authorization, "Token " + _apiKey);
                            req.set(http::field::user_agent, "DeepgramCppClient/1.0");
                        }));

                    _webSocket.handshake(_host, target);
                    std::cout << "WebSocket connected successfully!" << std::endl;
                    connected_ = true;
                    _keepReceiving = true;
                    return true;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Connection error: " << e.what() << std::endl;
                    return false;
                }
            }

            void startReceiving(std::function<void(const std::string &)> callBack)
            {
                _dataReceptionThread = std::thread([this, &callBack]()
                                            {
            std::cout << "Starting receive loop..." << std::endl;
            beast::flat_buffer buffer;
            beast::error_code ec;
            while (_keepReceiving && _webSocket.is_open())
            {
                try
                {   
                    size_t bytesRead = _webSocket.read(buffer, ec);
                    
                    if (ec == websocket::error::closed)
                    {
                        std::cout << "WebSocket closed by server." << std::endl;
                        break;
                    }
                    
                    if (ec)
                    {
                        std::cerr << "Read error: " << ec.message() << std::endl;
                        break;
                    }
                    
                    std::string message = beast::buffers_to_string(buffer.data());
                    callBack(message);
                    buffer.consume(bytesRead);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Exception in receive loop: " << e.what() << std::endl;
                    break;
                }
            }
            
            std::cout << "Receive loop ended." << std::endl; });
            }

            void startKeepalive()
            {
                _keepaliveThread = std::thread([this]()
                                               {
            std::cout << "Starting keepalive thread..." << std::endl;
            
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
                        std::cerr << "Keepalive send error: " << ec.message() << std::endl;
                        break;
                    }
                    
                    std::cout << "Sent keepalive message" << std::endl;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Keepalive error: " << e.what() << std::endl;
                    break;
                }
            }
            
            std::cout << "Keepalive thread ended." << std::endl; });
            }

            bool streamAudioFile(const std::vector<uint8_t> &audioData)
            {
                if (!connected_ || !_webSocket.is_open())
                {
                    std::cerr << "Not connected to Deepgram." << std::endl;
                    return false;
                }

                std::cout << "Streaming " << audioData.size() << " bytes of audio..." << std::endl;

                const size_t chunkSize = 4000;
                size_t offset = 0;
                int chunkNumber = 1;

                while (offset < audioData.size() && _webSocket.is_open())
                {
                    size_t currentChunkSize = std::min(chunkSize, audioData.size() - offset);

                    std::cout << "Sending chunk " << chunkNumber++ << " (" << currentChunkSize << " bytes)..." << std::endl;

                    if (!sendAudioChunk(audioData.data() + offset, currentChunkSize))
                    {
                        std::cerr << "Failed to send audio chunk." << std::endl;
                        return false;
                    }

                    offset += currentChunkSize;

                    // Send in real-time: 100ms chunks every 100ms
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                std::cout << "Finished streaming audio data." << std::endl;

                // Send close stream message
                sendCloseStream();

                return true;
            }

            bool sendFinalizeMessage()
            {
                if (!_webSocket.is_open())
                {
                    std::cerr << "websocket not open" << std::endl;
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
                        std::cerr << "Write error: " << ec.message() << std::endl;
                        return false;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "sendFinalizeMessage error: " << e.what() << std::endl;
                    return false;
                }
                return true;
            }

            bool sendAudioChunk(const uint8_t *data, size_t size)
            {
                try
                {
                    std::cout << "sending chunk...." << std::endl;
                    beast::error_code ec;
                    // Send as binary WebSocket message
                    _webSocket.binary(true);
                    int bytesOut = _webSocket.write(net::buffer(data, size), ec);
                    std::cout << "*** done sending chunk.... " <<  bytesOut << std::endl;

                    if (ec)
                    {
                        std::cerr << "Write error: " << ec.message() << std::endl;
                        return false;
                    }
                    return true;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Send chunk error: " << e.what() << std::endl;
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
                        std::cerr << "Error sending close stream: " << ec.message() << std::endl;
                    }
                    else
                    {
                        std::cout << "Sent close stream message." << std::endl;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error in sendCloseStream: " << e.what() << std::endl;
                }
            }

            void stopReceiving()
            {
                _keepReceiving = false;
            }

            void close()
            {
                std::cout << "Closing connection..." << std::endl;

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
                            std::cerr << "Close error: " << ec.message() << std::endl;
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "Exception during close: " << e.what() << std::endl;
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
                std::cout << "Connection closed." << std::endl;
            }
        };
    }
}