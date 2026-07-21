#pragma once

#include "../../include/deepgrampp/listen-ws.hpp"
#include "../../include/deepgrampp/transport/lws_websocket_transport.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace deepgram
{
    namespace listen
    {
        class ListenWebsocketClientImpl
        {
        public:
            ListenWebsocketClientImpl(const std::string &host,
                                       const std::string &apiKey,
                                       std::shared_ptr<transport::IWebSocketTransport> wsTransport)
                : _host(host), _apiKey(apiKey),
                  _wsTransport(wsTransport ? std::move(wsTransport)
                                            : std::make_shared<transport::LwsWebSocketTransport>())
            {
            }

            ~ListenWebsocketClientImpl()
            {
                close();
            }

            /**
             * Registered once at construction time (before connect() is ever called) so
             * no messages are missed between a successful connect() and a later
             * startReceiving() call, which the public API keeps for source compatibility
             * but no longer needs to arm the receive path.
             */
            void setHandlers(std::function<void(const std::string &)> onMessage,
                              std::function<void(const std::string &)> onError)
            {
                _wsTransport->setOnTextMessage(std::move(onMessage));
                _wsTransport->setOnError(std::move(onError));
            }

            bool connect(const LiveTranscriptionOptions &options)
            {
                if (_wsTransport->isOpen())
                {
                    spdlog::warn("Already connected to Deepgram.");
                    return true;
                }
                try
                {
                    transport::WebSocketConnectOptions wsOptions;
                    wsOptions.url = "wss://" + _host + options.toQueryString();
                    wsOptions.headers["Authorization"] = "Token " + _apiKey;
                    wsOptions.headers["User-Agent"] = "DeepgramCppClient/1.0";

                    spdlog::debug("Connecting to {} ...", wsOptions.url);
                    _wsTransport->connect(wsOptions);
                    spdlog::debug("WebSocket connected successfully!");
                    return true;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Connection error: {}", e.what());
                    return false;
                }
            }

            void startKeepalive()
            {
                _keepaliveThread = std::thread([this]()
                                               {
                spdlog::debug("Starting keepalive thread...");
                while (_wsTransport->isOpen())
                {
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    if (!_wsTransport->isOpen()) break;
                    try
                    {
                        _wsTransport->sendText(std::string(control::KEEPALIVE_MESSAGE));
                        spdlog::debug("Sent keepalive message");
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Keepalive error: {}", e.what());
                        break;
                    }
                }
                spdlog::debug("Keepalive thread ended."); });
            }

            bool streamAudio(const std::vector<uint8_t> &audioData, size_t chunkSize = 4096)
            {
                if (!_wsTransport->isOpen())
                {
                    spdlog::error("Not connected to Deepgram.");
                    return false;
                }
                spdlog::debug("Streaming {} bytes of audio...", audioData.size());

                size_t offset = 0;
                while (offset < audioData.size() && _wsTransport->isOpen())
                {
                    size_t currentChunkSize = std::min(chunkSize, audioData.size() - offset);
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

            bool sendFinalizeMessage()
            {
                return sendText(control::FINALIZE_MESSAGE);
            }

            bool sendAudioChunk(const uint8_t *data, size_t size)
            {
                if (!_wsTransport->isOpen())
                {
                    spdlog::error("can't send audio chunk, websocket not open");
                    return false;
                }
                try
                {
                    _wsTransport->sendBinary(std::vector<uint8_t>(data, data + size));
                    return true;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Send chunk error: {}", e.what());
                    return false;
                }
            }

            void sendCloseStream()
            {
                sendText(control::CLOSE_MESSAGE);
            }

            void stopReceiving()
            {
                // No-op: message delivery is tied to the underlying transport's
                // lifetime and stops automatically once close() is called.
            }

            void close()
            {
                if (!_wsTransport->isOpen())
                {
                    return;
                }
                spdlog::debug("Closing connection...");

                sendCloseStream();
                // Wait a moment for final messages
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                _wsTransport->close();

                if (_keepaliveThread.joinable())
                {
                    _keepaliveThread.join();
                }
                spdlog::debug("Connection closed.");
            }

        private:
            bool sendText(const std::string &message)
            {
                if (!_wsTransport->isOpen())
                {
                    spdlog::error("can't send message, websocket not open");
                    return false;
                }
                try
                {
                    _wsTransport->sendText(message);
                    return true;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("sendText error: {}", e.what());
                    return false;
                }
            }

            std::string _host;
            std::string _apiKey;
            std::shared_ptr<transport::IWebSocketTransport> _wsTransport;
            std::thread _keepaliveThread;
        };
    }
}
