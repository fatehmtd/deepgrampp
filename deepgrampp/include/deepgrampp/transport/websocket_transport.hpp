#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace deepgram
{
    namespace transport
    {
        struct WebSocketConnectOptions
        {
            std::string url;
            std::map<std::string, std::string> headers;
        };

        /**
         * Event-driven WebSocket transport interface.
         * Register setOn* handlers before connect(). sendText/sendBinary/close/isOpen
         * are thread-safe; handler callbacks fire from the internal receive-loop thread.
         */
        class IWebSocketTransport
        {
        public:
            using OpenHandler = std::function<void()>;
            using TextMessageHandler = std::function<void(const std::string &)>;
            using BinaryMessageHandler = std::function<void(const std::vector<std::uint8_t> &)>;
            using ErrorHandler = std::function<void(const std::string &)>;
            using CloseHandler = std::function<void()>;

            virtual ~IWebSocketTransport() = default;

            virtual void setOnOpen(OpenHandler handler) = 0;
            virtual void setOnTextMessage(TextMessageHandler handler) = 0;
            virtual void setOnBinaryMessage(BinaryMessageHandler handler) = 0;
            virtual void setOnError(ErrorHandler handler) = 0;
            virtual void setOnClose(CloseHandler handler) = 0;

            /**
             * Blocks until the WebSocket handshake completes or throws.
             * Throws std::runtime_error on DNS, TLS, or protocol errors.
             */
            virtual void connect(const WebSocketConnectOptions &options) = 0;

            virtual void sendText(const std::string &message) = 0;
            virtual void sendBinary(const std::vector<std::uint8_t> &payload) = 0;

            virtual void close() = 0;
            virtual bool isOpen() const = 0;
        };

    } // namespace transport
} // namespace deepgram
