#pragma once

#include "listen.hpp"
#include "deepgram.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/beast/http.hpp>

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <nlohmann/json.hpp>

namespace deepgram
{
    using Websocket = boost::beast::websocket::stream<boost::beast::net::ssl::stream<boost::asio::ip::tcp::socket>>;

    namespace listen
    {
        class ListenWebsocketClient
        {
        public:
            /**
             * @brief Constructs a WebSocket client for Deepgram's Listen API.
             *
             * @param host The hostname of the Deepgram API server (e.g., "api.deepgram.com").
             * @param apiKey Your Deepgram API key for authentication.
             * @param port The port to connect to (default is "443" for HTTPS).
             *
             * This constructor initializes the WebSocket client with the provided host, API key, and port.
             * It sets up the SSL context and prepares the WebSocket stream for communication.
             */
            ListenWebsocketClient(const std::string &host, const std::string &apiKey, const std::string &port = "443");
            ~ListenWebsocketClient();

            /**
             * @brief Connects to the Deepgram WebSocket server.
             *
             * @return true if the connection was successful, false otherwise.
             *
             * This method resolves the host and port, performs the SSL handshake, and establishes the WebSocket connection.
             * It also sets the necessary headers for authentication.
             * After a successful connection, it sets the `connected_` flag to true and starts the receiving loop.
             *
             * @note This method should be called before starting to send or receive messages.
             */
            bool connect(const LiveTranscriptionOptions& options);

            void startReceiving();
            void startKeepalive();
            bool streamAudioFile(const std::vector<uint8_t> &audioData);

            using PartialTranscriptionCallback = std::function<void(const TranscriptionResult &)>;
            using FinalTranscriptionCallback = std::function<void(const TranscriptionResult &)>;
            using MetadataCallback = std::function<void(const nlohmann::json &)>;
            using ErrorCallback = std::function<void(const std::string &)>;
            using SpeechStartedCallback = std::function<void()>;
            using UtteranceEndCallback = std::function<void(const UtteranceEnd&)>;

            void setOnPartialTranscription(PartialTranscriptionCallback cb);
            void setOnFinalTranscription(FinalTranscriptionCallback cb);
            void setOnMetadata(MetadataCallback cb);
            void setOnError(ErrorCallback cb);
            void setOnSpeechStarted(SpeechStartedCallback cb);
            void setUtteranceEndCallback(UtteranceEndCallback cb);

            void stopReceiving();
            void close();

        private:
            bool sendAudioChunk(const uint8_t *data, size_t size);

            void sendCloseStream();

            void handleResponse(const std::string &message);

        private:
            std::string host_;
            std::string apiKey_;
            std::string port_;
            boost::asio::io_context ioContext_;
            boost::beast::net::ssl::context sslContext_;
            Websocket ws_;
            std::thread workerThread_;
            std::thread keepaliveThread_;
            std::atomic<bool> keepReceiving_;
            std::atomic<bool> connected_;

            PartialTranscriptionCallback onPartialTranscription_ = [](const TranscriptionResult &) {};
            FinalTranscriptionCallback onFinalTranscription_ = [](const TranscriptionResult &) {};
            MetadataCallback onMetadata_ = [](const nlohmann::json &) {};
            ErrorCallback onError_ = [](const std::string &) {};
            UtteranceEndCallback onUtteranceEnd_ = [](const UtteranceEnd&){};
            SpeechStartedCallback onSpeechStarted_ = [](const SpeechStarted&) {};
        };

    }
}