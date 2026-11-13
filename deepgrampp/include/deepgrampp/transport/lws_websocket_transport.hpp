#pragma once

#include <deepgrampp_lib_export.h>
#include "websocket_transport.hpp"

#include <memory>
#include <string>

namespace deepgram
{
    namespace transport
    {
        struct LwsWebSocketTransportImpl;

        /**
         * WebSocket transport backed by libwebsockets.
         * Used as the default transport for ListenWebsocketClient and SpeakWebsocketClient
         * unless a custom transport is injected via their constructors.
         */
        class DEEPGRAMPP_EXPORT LwsWebSocketTransport final : public IWebSocketTransport
        {
        public:
            /**
             * @param caFilePath Path to a PEM-encoded CA bundle to use for verifying
             *        server certificates. The bundled mbedTLS backend has no
             *        visibility into the OS trust store on any platform, so TLS
             *        connections need one; empty uses libwebsockets' default (no
             *        trust anchors).
             */
            explicit LwsWebSocketTransport(std::string caFilePath = {});
            ~LwsWebSocketTransport() override;

            void setOnOpen(OpenHandler handler) override;
            void setOnTextMessage(TextMessageHandler handler) override;
            void setOnBinaryMessage(BinaryMessageHandler handler) override;
            void setOnError(ErrorHandler handler) override;
            void setOnClose(CloseHandler handler) override;

            void connect(const WebSocketConnectOptions &options) override;
            void sendText(const std::string &message) override;
            void sendBinary(const std::vector<std::uint8_t> &payload) override;
            void close() override;
            bool isOpen() const override;

        private:
            std::unique_ptr<LwsWebSocketTransportImpl> _impl;
        };

    } // namespace transport
} // namespace deepgram
