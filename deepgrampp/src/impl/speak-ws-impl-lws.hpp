#pragma once

#include "../../include/deepgrampp/speak-ws.hpp"
#include "../../include/deepgrampp/transport/lws_websocket_transport.hpp"

#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace deepgram
{
    namespace speak
    {
        class SpeakWebsocketClientImpl
        {
        public:
            SpeakWebsocketClientImpl(const std::string &host,
                                      const std::string &apiKey,
                                      std::shared_ptr<transport::IWebSocketTransport> wsTransport,
                                      const std::string &caFilePath = {})
                : _host(host), _apiKey(apiKey),
                  _wsTransport(wsTransport ? std::move(wsTransport)
                                            : std::make_shared<transport::LwsWebSocketTransport>(caFilePath))
            {
            }

            ~SpeakWebsocketClientImpl()
            {
                close();
            }

            bool isConnected() const
            {
                return _wsTransport->isOpen();
            }

            void setSpeechReceptionTimeout(int timeoutMs)
            {
                if (timeoutMs > 0)
                {
                    _speechReceptionTimeoutMs = timeoutMs;
                }
                else
                {
                    spdlog::warn("Invalid speech reception timeout value: {}. It must be greater than 0.", timeoutMs);
                    _speechReceptionTimeoutMs = 500;
                }
            }

            /**
             * Registered once at construction time (before connect() is ever called) so
             * no messages are missed between a successful connect() and a later
             * startReceiving() call, which the public API keeps for source compatibility
             * but no longer needs to arm the receive path.
             */
            void setHandlers(std::function<void(const char *, int)> onAudio,
                              std::function<void(const std::string &)> onText,
                              std::function<void(const std::string &)> onError,
                              std::function<void()> onDisconnected,
                              std::function<void()> onSpeechStarted)
            {
                _wsTransport->setOnBinaryMessage([this, onAudio, onSpeechStarted](const std::vector<std::uint8_t> &data)
                                                 {
                    if (!_receivingSpeech.exchange(true))
                    {
                        if (onSpeechStarted) onSpeechStarted();
                    }
                    if (onAudio)
                    {
                        onAudio(reinterpret_cast<const char *>(data.data()), static_cast<int>(data.size()));
                    }
                    _lastSpeechMessageTime.store(nowMs()); });
                _wsTransport->setOnTextMessage(std::move(onText));
                _wsTransport->setOnError(std::move(onError));
                _wsTransport->setOnClose(std::move(onDisconnected));
            }

            /**
             * Starts the background monitor that infers end-of-speech from a gap in
             * binary audio frames (Deepgram does not send an explicit "speech ended"
             * event). Must be started only once the connection is open.
             */
            void startSpeechTimeoutMonitor(std::function<void()> onSpeechEnded)
            {
                _timeoutThread = std::thread([this, onSpeechEnded]()
                                             {
                    try
                    {
                        while (_wsTransport->isOpen())
                        {
                            if (_receivingSpeech.load())
                            {
                                uint64_t currentTime = nowMs();
                                uint64_t lastTime = _lastSpeechMessageTime.load();
                                if (currentTime - lastTime > static_cast<uint64_t>(_speechReceptionTimeoutMs))
                                {
                                    spdlog::debug("No speech data received for {} milliseconds, assuming end of speech.", _speechReceptionTimeoutMs);
                                    _receivingSpeech.store(false);
                                    if (onSpeechEnded) onSpeechEnded();
                                }
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        }
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Exception in timeout thread: {}", e.what());
                    } });
            }

            bool connect(const LiveSpeakConfig &config)
            {
                try
                {
                    transport::WebSocketConnectOptions wsOptions;
                    wsOptions.url = "wss://" + _host + config.toQueryString();
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

            bool sendPayload(const std::string &payload)
            {
                if (!_wsTransport->isOpen())
                {
                    spdlog::error("can't send payload, websocket not open");
                    return false;
                }
                try
                {
                    _wsTransport->sendText(payload);
                    return true;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("sendPayload error: {}", e.what());
                    return false;
                }
            }

            bool sendCloseStream()
            {
                return sendPayload(std::string(control::CLOSE));
            }

            void close()
            {
                if (_wsTransport->isOpen())
                {
                    spdlog::debug("Closing connection...");

                    sendCloseStream();
                    // Wait a moment for final messages
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));

                    _wsTransport->close();
                }

                // The timeout monitor thread's own loop exits once isOpen() goes
                // false (whether that happened here or the socket was already
                // closed, e.g. by the server), so it must always be joined here --
                // otherwise a joinable std::thread left in the destructor calls
                // std::terminate().
                if (_timeoutThread.joinable())
                {
                    _timeoutThread.join();
                }
                spdlog::debug("Connection closed.");
            }

        private:
            static uint64_t nowMs()
            {
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now().time_since_epoch())
                    .count();
            }

            std::string _host;
            std::string _apiKey;
            std::shared_ptr<transport::IWebSocketTransport> _wsTransport;
            std::thread _timeoutThread;
            std::atomic<bool> _receivingSpeech{false};
            std::atomic<uint64_t> _lastSpeechMessageTime{0};
            int _speechReceptionTimeoutMs = 500;
        };
    }
}
