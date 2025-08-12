
#include "listen-ws.hpp"

using namespace deepgram::listen;

using namespace boost;
using namespace boost::beast;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using Websocket = websocket::stream<ssl::stream<tcp::socket>>;

ListenWebsocketClient::ListenWebsocketClient(const std::string &host,
                                             const std::string &apiKey,
                                             const std::string &port)
    : host_(host), apiKey_(apiKey), port_(port),
      ioContext_(), sslContext_(boost::beast::net::ssl::context::tlsv12_client),
      ws_(ioContext_, sslContext_), keepReceiving_(false), connected_(false)
{
    sslContext_.set_default_verify_paths();
    sslContext_.set_verify_mode(boost::beast::net::ssl::verify_peer);
}

ListenWebsocketClient::~ListenWebsocketClient()
{
    close();
}

bool ListenWebsocketClient::connect(const LiveTranscriptionOptions& options)
{
    try
    {
        std::string target = options.toQueryString();

        std::cout << "target: " << target << std::endl;

        tcp::resolver resolver{ioContext_};
        auto const results = resolver.resolve(host_, port_);

        std::cout << "Connecting to " << host_ << ":" << port_ << "..." << std::endl;
        net::connect(ws_.next_layer().lowest_layer(), results.begin(), results.end());

        std::cout << "Performing SSL handshake..." << std::endl;
        ws_.next_layer().set_verify_callback(ssl::host_name_verification(host_));
        ws_.next_layer().handshake(ssl::stream_base::client);

        std::cout << "Performing WebSocket handshake..." << std::endl;
        ws_.set_option(websocket::stream_base::decorator(
            [this](websocket::request_type &req)
            {
                req.set(http::field::authorization, "Token " + apiKey_);
                req.set(http::field::user_agent, "DeepgramCppClient/1.0");
            }));

        ws_.handshake(host_, target);
        std::cout << "WebSocket connected successfully!" << std::endl;

        connected_ = true;
        keepReceiving_ = true;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Connection error: " << e.what() << std::endl;
        return false;
    }
}

void ListenWebsocketClient::startReceiving()
{
    workerThread_ = std::thread([this]()
                                {
            std::cout << "Starting receive loop..." << std::endl;
            beast::flat_buffer buffer;
            beast::error_code ec;
            while (keepReceiving_ && ws_.is_open())
            {
                try
                {   
                    size_t bytesRead = ws_.read(buffer, ec);
                    
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
                    handleResponse(message);
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

void ListenWebsocketClient::startKeepalive()
{
    keepaliveThread_ = std::thread([this]()
                                   {
            std::cout << "Starting keepalive thread..." << std::endl;
            
            while (keepReceiving_ && ws_.is_open())
            {
                std::this_thread::sleep_for(std::chrono::seconds(5)); // Send every 5 seconds
                
                if (!keepReceiving_ || !ws_.is_open()) break;
                
                try
                {
                    beast::error_code ec;
                    ws_.write(net::buffer(std::string(deepgram::control::KEEPALIVE_MESSAGE)), ec);
                    
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

bool ListenWebsocketClient::streamAudioFile(const std::vector<uint8_t> &audioData)
{
    if (!connected_ || !ws_.is_open())
    {
        std::cerr << "Not connected to Deepgram." << std::endl;
        return false;
    }

    std::cout << "Streaming " << audioData.size() << " bytes of audio..." << std::endl;

    const size_t chunkSize = 4000;
    size_t offset = 0;
    int chunkNumber = 1;

    while (offset < audioData.size() && ws_.is_open())
    {
        size_t currentChunkSize = std::min(chunkSize, audioData.size() - offset);

        // std::cout << "Sending chunk " << chunkNumber++ << " (" << currentChunkSize << " bytes)..." << std::endl;

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

bool ListenWebsocketClient::sendAudioChunk(const uint8_t *data, size_t size)
{
    try
    {
        beast::error_code ec;
        // Send as binary WebSocket message
        ws_.binary(true);
        ws_.write(net::buffer(data, size), ec);

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

void ListenWebsocketClient::sendCloseStream()
{
    try
    {
        // Send as text message
        ws_.binary(false);
        beast::error_code ec;
        ws_.write(net::buffer(std::string(deepgram::control::CLOSE_MESSAGE)), ec);

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

void ListenWebsocketClient::handleResponse(const std::string &message)
{
    try
    {
        auto json = nlohmann::json::parse(message);

        auto transcriptionResult = deepgram::listen::TranscriptionResult::fromJson(json);

        if (transcriptionResult.type == "Results")
        {
            if (transcriptionResult.isFinal || transcriptionResult.speech_final)
            {
                onFinalTranscription_(transcriptionResult);
            }
            else
            {
                onPartialTranscription_(transcriptionResult);
            }
        }
        else if (transcriptionResult.type == "Metadata")
        {
            onMetadata_(json);
        }
        else if (transcriptionResult.type == "UtteranceEnd")
        {
            deepgram::listen::UtteranceEnd utteranceEnd = deepgram::listen::UtteranceEnd::fromJson(json);
            utteranceEnd.print();
        }
        else if (transcriptionResult.type == "SpeechStarted")
        {
            deepgram::listen::SpeechStarted speechStarted = deepgram::listen::SpeechStarted::fromJson(json);
            speechStarted.print();
            onSpeechStarted_();
        }
        else
        {
            std::cout << "Received unknown message type: " << transcriptionResult.type << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error handling response: " << e.what() << std::endl;
        std::cout << "Raw message: " << message << std::endl;
    }
}

void ListenWebsocketClient::stopReceiving()
{
    keepReceiving_ = false;
}

void ListenWebsocketClient::close()
{
    std::cout << "Closing connection..." << std::endl;

    stopReceiving();

    if (connected_ && ws_.is_open())
    {
        sendCloseStream();

        // Wait a moment for final messages
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        try
        {
            beast::error_code ec;
            ws_.close(websocket::close_code::normal, ec);
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

    if (workerThread_.joinable())
    {
        workerThread_.join();
    }

    if (keepaliveThread_.joinable())
    {
        keepaliveThread_.join();
    }

    connected_ = false;
    std::cout << "Connection closed." << std::endl;
}

void ListenWebsocketClient::setOnPartialTranscription(PartialTranscriptionCallback cb)
{
    onPartialTranscription_ = std::move(cb);
}

void ListenWebsocketClient::setOnFinalTranscription(FinalTranscriptionCallback cb)
{
    onFinalTranscription_ = std::move(cb);
}

void ListenWebsocketClient::setOnMetadata(MetadataCallback cb)
{
    onMetadata_ = std::move(cb);
}

void ListenWebsocketClient::setOnError(ErrorCallback cb)
{
    onError_ = std::move(cb);
}

void ListenWebsocketClient::setOnSpeechStarted(SpeechStartedCallback cb)
{
    onSpeechStarted_ = std::move(cb);
}

void deepgram::listen::ListenWebsocketClient::setUtteranceEndCallback(UtteranceEndCallback cb)
{
    onUtteranceEnd_ = std::move(cb);
}
