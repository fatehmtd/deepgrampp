#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/beast/http.hpp>

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <functional>
#include "deepgram.hpp"
#include "listen.hpp"
#include "speak.hpp"

using namespace boost;
using namespace boost::beast;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using Websocket = websocket::stream<ssl::stream<tcp::socket>>;

const std::string apiKey = "da3313d72d69f139a4d19f1b19fd0848ab22fdfa";

std::vector<uint8_t> readAudioFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        throw std::runtime_error("Failed to open audio file: " + filePath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        throw std::runtime_error("Failed to read audio file");
    }

    std::cout << "Successfully read " << size << " bytes from audio file" << std::endl;
    return buffer;
}

class ListenWebsocketClient
{
public:
    ListenWebsocketClient(const std::string &host, const std::string &apiKey, const std::string &port = "443")
        : host_(host), apiKey_(apiKey), port_(port),
          ioContext_(), sslContext_(ssl::context::tlsv12_client),
          ws_(ioContext_, sslContext_), keepReceiving_(false), connected_(false)
    {
        sslContext_.set_default_verify_paths();
        sslContext_.set_verify_mode(ssl::verify_peer);
    }

    ~ListenWebsocketClient()
    {
        close();
    }

    bool connect()
    {
        try
        {
            deepgram::listen::LiveTranscriptionOptions options({.model = deepgram::listen::models::nova_3::GENERAL,
                                                                .language = deepgram::listen::languages::nova_3::MULTILINGUAL,
                                                                .sampleRate = 16000,
                                                                .encoding = deepgram::listen::encoding::LINEAR_16});

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

    void startReceiving()
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

    void startKeepalive()
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

    bool streamAudioFile(const std::vector<uint8_t> &audioData)
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

private:
    bool sendAudioChunk(const uint8_t *data, size_t size)
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

    void sendCloseStream()
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

    void handleResponse(const std::string &message)
    {
        try
        {
            auto json = nlohmann::json::parse(message);

            // std::cout << "Received message: " << message << std::endl;

            auto transcriptionResult = deepgram::listen::TranscriptionResult::fromJson(json);
            // transcriptionResult.print();

            if (transcriptionResult.type == "Results")
            {
                if (transcriptionResult.isFinal)
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
            std::cout << "--------------" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error handling response: " << e.what() << std::endl;
            std::cout << "Raw message: " << message << std::endl;
        }
    }

    void stopReceiving()
    {
        keepReceiving_ = false;
    }

    void close()
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

    using PartialTranscriptionCallback = std::function<void(const deepgram::listen::TranscriptionResult &)>;
    using FinalTranscriptionCallback = std::function<void(const deepgram::listen::TranscriptionResult &)>;
    using MetadataCallback = std::function<void(const nlohmann::json &)>;
    using ErrorCallback = std::function<void(const std::string &)>;
    using SpeechStartedCallback = std::function<void()>;

    void setOnPartialTranscription(PartialTranscriptionCallback cb) { onPartialTranscription_ = std::move(cb); }
    void setOnFinalTranscription(FinalTranscriptionCallback cb) { onFinalTranscription_ = std::move(cb); }
    void setOnMetadata(MetadataCallback cb) { onMetadata_ = std::move(cb); }
    void setOnError(ErrorCallback cb) { onError_ = std::move(cb); }
    void setOnSpeechStarted(SpeechStartedCallback cb) { onSpeechStarted_ = std::move(cb); }

private:
    std::string host_;
    std::string apiKey_;
    std::string port_;
    net::io_context ioContext_;
    ssl::context sslContext_;
    Websocket ws_;
    std::thread workerThread_;
    std::thread keepaliveThread_;
    std::atomic<bool> keepReceiving_;
    std::atomic<bool> connected_;

    PartialTranscriptionCallback onPartialTranscription_ = [](const deepgram::listen::TranscriptionResult &)
    {
        std::cout << "partial transcription received" << std::endl;
    };
    FinalTranscriptionCallback onFinalTranscription_ = [](const deepgram::listen::TranscriptionResult &)
    {
        std::cout << "final transcription received" << std::endl;
    };
    MetadataCallback onMetadata_ = [](const nlohmann::json &)
    {
        std::cout << "metada received" << std::endl;
    };
    ErrorCallback onError_ = [](const std::string &)
    {
        std::cout << "error received" << std::endl;
    };
    SpeechStartedCallback onSpeechStarted_ = []()
    { std::cout << "Speech started" << std::endl; };
};

int main()
{
    try
    {
        // Read the audio file
        std::cout << "Reading audio file..." << std::endl;
        auto audioData = readAudioFile("/Users/fateh/Documents/recording.raw");

        if (audioData.empty())
        {
            std::cerr << "Error: Audio file is empty." << std::endl;
            return EXIT_FAILURE;
        }

        // Create and connect the client
        ListenWebsocketClient client("api.deepgram.com", apiKey);

        if (!client.connect())
        {
            std::cerr << "Failed to connect to Deepgram." << std::endl;
            return EXIT_FAILURE;
        }

        // Start receiving messages
        client.startReceiving();

        // Wait for connection to stabilize and receive metadata
        std::cout << "Waiting for metadata..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Stream the audio file
        if (!client.streamAudioFile(audioData))
        {
            std::cerr << "Failed to stream audio file." << std::endl;
            return EXIT_FAILURE;
        }

        // Wait for final transcription results
        std::cout << "Waiting for final results..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::cout << "Transcription complete!" << std::endl;
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}