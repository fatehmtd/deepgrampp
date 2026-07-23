#pragma once

#include <deepgrampp_lib_export.h>
#include "speak.hpp"
#include "transport/websocket_transport.hpp"
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
            /**
             * @param apiKey Your Deepgram API key for authentication.
             * @param wsTransport Optional WebSocket transport to use instead of the default
             *        libwebsockets-based transport (deepgram::transport::LwsWebSocketTransport).
             *        Useful for tests or to plug in a different networking stack.
             * @param caFilePath Path to a PEM-encoded CA bundle passed to the default
             *        transport when `wsTransport` is null. Needed on platforms where
             *        the bundled mbedTLS backend has no visibility into the OS trust
             *        store (e.g. Android); leave empty elsewhere. Ignored if
             *        `wsTransport` is provided.
             */
            SpeakWebsocketClient(const std::string &apiKey,
                                  std::shared_ptr<transport::IWebSocketTransport> wsTransport = nullptr,
                                  const std::string &caFilePath = {});
            ~SpeakWebsocketClient();

            /**
             * Connects to the WebSocket server.
             * This method establishes a connection to the Deepgram WebSocket server.
             * Audio/message delivery begins automatically once this returns successfully.
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
             * @deprecated No longer required: audio/message delivery starts automatically
             * once connect() succeeds. Kept as a no-op for source compatibility.
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

            /**
             * Callback type definitions
             */
            using SpeechResultCallback = std::function<void(const char *, int)>;
            using SpeechControlResponseCallback = std::function<void(const SpeakControlResponse &)>;
            using SpeechCloseFrameCallback = std::function<void(const SpeakCloseFrame &)>;
            using SpeechMetadataResponseCallback = std::function<void(const MetadataResponse &)>;
            using SpeechErrorCallback = std::function<void(const std::string &)>;
            using SpeechDisconnectedCallback = std::function<void()>;
            using SpeechStartedCallback = std::function<void()>;
            using SpeechEndedCallback = std::function<void()>;

            void setSpeechResultCallback(SpeechResultCallback callback);
            void setSpeechControlResponseCallback(SpeechControlResponseCallback callback);
            void setSpeechCloseFrameCallback(SpeechCloseFrameCallback callback);
            void setSpeechMetadataResponseCallback(SpeechMetadataResponseCallback callback);

            void setSpeechErrorCallback(SpeechErrorCallback callback);
            void setSpeechDisconnectedCallback(SpeechDisconnectedCallback callback);

            void setSpeechStartedCallback(SpeechStartedCallback callback);
            void setSpeechEndedCallback(SpeechEndedCallback callback);

            /**
             * Sets the speech reception timeout.
             * This timeout defines how long the client will wait for speech data before considering the connection lost
             * @note this is not officially supported by Deepgram, but it is useful to detect speech generation end based on inactivity.
             * @param timeoutMs Timeout in milliseconds. Default is 30000 ms (30 seconds)
             */
            void setSpeechReceptionTimeout(int timeoutMs);

        private:
            SpeechResultCallback _speechResultCallback;
            SpeechControlResponseCallback _speechControlResponseCallback;
            SpeechCloseFrameCallback _speechCloseFrameCallback;
            SpeechMetadataResponseCallback _speechMetadataResponseCallback;
            std::unique_ptr<SpeakWebsocketClientImpl> _speakWebsocketClientImpl;
            SpeechErrorCallback _speechErrorCallback;
            SpeechDisconnectedCallback _speechDisconnectedCallback;
            SpeechStartedCallback _speechStartedCallback;
            SpeechEndedCallback _speechEndedCallback;
        };
    }
}