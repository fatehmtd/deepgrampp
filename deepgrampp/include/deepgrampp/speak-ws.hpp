#pragma once

#include <deepgrampp_lib_export.h>
#include "speak.hpp"
#include <functional>
#include <memory>

namespace deepgram
{
    namespace speak
    {
        /**
         * Implementation details for the WebSocket client.
         */
        class SpeakWebsocketClientImpl;

        /**
         * WebSocket client for Deepgram's speech API.
         * This class provides method to synthesize speech from text in a streaming fashion.
         */
        class DEEPGRAMPP_EXPORT SpeakWebsocketClient
        {
        public:
            SpeakWebsocketClient(const std::string &host, const std::string &apiKey, const std::string &port = "443");
            ~SpeakWebsocketClient();

            /**
             * Connects to the WebSocket server.
             * This method establishes a connection to the Deepgram WebSocket server.
             * @param config The configuration for the live speech session.
             * @return True if the connection was successful, false otherwise.
             */
            bool connect(const LiveSpeakConfig &config);

            /**
             * Closes the WebSocket connection.
             * This method terminates the connection to the Deepgram WebSocket server.
             */
            void close();

            /**
             * Starts receiving audio data from the WebSocket.
             * This method begins the process of receiving audio data from the Deepgram WebSocket server.
             */
            void startReceiving();

            /**
             * Sends a text message to the WebSocket.
             * This method sends a text message to the Deepgram WebSocket server for processing.
             */
            bool speak(const std::string &text);

            /**
             * Sends a flush message to the WebSocket.
             * This method sends a flush message to the Deepgram WebSocket server.
             */
            bool sendFlushMessage();

            /**
             * Sends a clear message to the WebSocket.
             * This method sends a clear message to the Deepgram WebSocket server.
             */
            bool sendClearMessage();

            /**
             * Sends a close message to the WebSocket.
             * This method sends a close message to the Deepgram WebSocket server.
             */
            bool sendCloseMessage();

            using SpeechResultCallback = std::function<void(const char *, int)>;
            using SpeechControlResponseCallback = std::function<void(const SpeakControlResponse &)>;
            using SpeechCloseFrameCallback = std::function<void(const SpeakCloseFrame &)>;
            using SpeechMetadataResponseCallback = std::function<void(const MetadataResponse &)>;

            void setSpeechResultCallback(SpeechResultCallback callback);
            void setSpeechControlResponseCallback(SpeechControlResponseCallback callback);
            void setSpeechCloseFrameCallback(SpeechCloseFrameCallback callback);
            void setSpeechMetadataResponseCallback(SpeechMetadataResponseCallback callback);

        private:
            SpeechResultCallback _speechResultCallback;
            SpeechControlResponseCallback _speechControlResponseCallback;
            SpeechCloseFrameCallback _speechCloseFrameCallback;
            SpeechMetadataResponseCallback _speechMetadataResponseCallback;
            std::unique_ptr<SpeakWebsocketClientImpl> _speakWebsocketClientImpl;
        };
    }
}