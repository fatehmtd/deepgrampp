#pragma once

#include "../../include/deepgrampp/listen-flux.hpp"
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
        namespace flux
        {
            class ListenFluxClientImpl
            {
            public:
                ListenFluxClientImpl(const std::string &host,
                                      const std::string &apiKey,
                                      std::shared_ptr<transport::IWebSocketTransport> wsTransport,
                                      const std::string &caFilePath = {})
                    : _host(host), _apiKey(apiKey),
                      _wsTransport(wsTransport ? std::move(wsTransport)
                                                : std::make_shared<transport::LwsWebSocketTransport>(caFilePath))
                {
                }

                ~ListenFluxClientImpl()
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

                bool connect(const FluxQueryParams &params)
                {
                    if (_wsTransport->isOpen())
                    {
                        spdlog::warn("Already connected to Deepgram.");
                        return true;
                    }
                    try
                    {
                        transport::WebSocketConnectOptions wsOptions;
                        wsOptions.url = "wss://" + _host + params.toQueryString();
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

                void stopReceiving()
                {
                    // No-op: message delivery is tied to the underlying transport's
                    // lifetime and stops automatically once close() is called.
                }

                bool streamAudio(const std::vector<uint8_t> &audioData, size_t chunkSize = 4000)
                {
                    if (!_wsTransport->isOpen())
                    {
                        spdlog::error("Not connected to Deepgram.");
                        return false;
                    }
                    size_t offset = 0;
                    while (offset < audioData.size() && _wsTransport->isOpen())
                    {
                        size_t currentChunkSize = std::min(chunkSize, audioData.size() - offset);
                        if (!sendAudioChunk(audioData.data() + offset, currentChunkSize))
                        {
                            spdlog::error("Failed to send audio chunk at offset {} of total {}", offset, audioData.size());
                            return false;
                        }
                        offset += currentChunkSize;
                    }
                    return true;
                }

                bool sendAudioChunk(const uint8_t *data, size_t size)
                {
                    if (!_wsTransport->isOpen())
                    {
                        spdlog::error("Not connected to Deepgram.");
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
                    if (!_wsTransport->isOpen())
                    {
                        spdlog::warn("WebSocket is not connected.");
                        return;
                    }
                    try
                    {
                        nlohmann::json jsonPayload;
                        jsonPayload["type"] = "CloseStream";
                        _wsTransport->sendText(jsonPayload.dump());
                        spdlog::debug("Sent close stream message.");
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Error in sendCloseStream: {}", e.what());
                    }
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
                    spdlog::debug("Connection closed.");
                }

            private:
                std::string _host;
                std::string _apiKey;
                std::shared_ptr<transport::IWebSocketTransport> _wsTransport;
            };
        }
    }
}
