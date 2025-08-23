#pragma once

#include <deepgrampp_lib_export.h>
#include "listen.hpp"
#include "deepgram.hpp"

#include <nlohmann/json.hpp>

#include <memory>

namespace deepgram
{
    namespace listen
    {
        /**
         * Control messages for the Deepgram WebSocket API.
         *
         * @note These messages are used to control the WebSocket connection and stream.
         */
        namespace control
        {
            constexpr const char *CLOSE_MESSAGE = R"({"type": "CloseStream"})";
            constexpr const char *KEEPALIVE_MESSAGE = R"({"type": "KeepAlive"})";
            constexpr const char *FINALIZE_MESSAGE = R"({"type": "Finalize"})";
        }

        class ListenWebsocketClientImpl;

        class DEEPGRAMPP_EXPORT ListenWebsocketClient
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
            ListenWebsocketClient(const std::string &apiKey);
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
            bool connect(const LiveTranscriptionOptions &options);

            void startReceiving();
            void startKeepalive();
            bool streamAudio(const std::vector<uint8_t> &audioData, int chunkSize=4096);

            /**
             * Use the Finalize message to flush the WebSocket stream.
             * This forces the server to immediately process any unprocessed audio data and return the final transcription results.
             */
            bool sendFinalizeMessage();

            using PartialTranscriptionCallback = std::function<void(const TranscriptionResult &)>;
            using FinalTranscriptionCallback = std::function<void(const TranscriptionResult &)>;
            using MetadataCallback = std::function<void(const nlohmann::json &)>;
            using ErrorCallback = std::function<void(const std::string &)>;
            using SpeechStartedCallback = std::function<void(const SpeechStarted &)>;
            using UtteranceEndCallback = std::function<void(const UtteranceEnd &)>;

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
            std::unique_ptr<ListenWebsocketClientImpl> websocketClientImpl_;

            PartialTranscriptionCallback onPartialTranscription_ = [](const TranscriptionResult &) {};
            FinalTranscriptionCallback onFinalTranscription_ = [](const TranscriptionResult &) {};
            MetadataCallback onMetadata_ = [](const nlohmann::json &) {};
            ErrorCallback onError_ = [](const std::string &) {};
            UtteranceEndCallback onUtteranceEnd_ = [](const UtteranceEnd &) {};
            SpeechStartedCallback onSpeechStarted_ = [](const SpeechStarted &) {};
        };

    }
}