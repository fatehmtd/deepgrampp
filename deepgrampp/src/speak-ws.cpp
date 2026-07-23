#include "impl/speak-ws-impl-lws.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "speak-ws.hpp"

deepgram::speak::SpeakWebsocketClient::SpeakWebsocketClient(const std::string &apiKey,
                                                              std::shared_ptr<transport::IWebSocketTransport> wsTransport,
                                                              const std::string &caFilePath)
{
    _speakWebsocketClientImpl = std::make_unique<SpeakWebsocketClientImpl>("api.deepgram.com", apiKey, std::move(wsTransport), caFilePath);
    // Wired up now, before connect() is ever called, so no messages are missed.
    _speakWebsocketClientImpl->setHandlers(
        [this](const char *data, int size)
        {
            if (_speechResultCallback)
            {
                _speechResultCallback(data, size);
            }
        },
        [this](const std::string &message)
        {
            nlohmann::json jsonResponse = nlohmann::json::parse(message);
            if (jsonResponse.contains("type"))
            {
                if (jsonResponse["type"] == "Metadata")
                {
                    if (_speechMetadataResponseCallback)
                    {
                        _speechMetadataResponseCallback(MetadataResponse::fromJson(jsonResponse));
                    }
                }
                else if (_speechControlResponseCallback)
                {
                    _speechControlResponseCallback(SpeakControlResponse::fromJson(jsonResponse));
                }
            }
            else if (_speechCloseFrameCallback)
            {
                _speechCloseFrameCallback(SpeakCloseFrame::fromJson(jsonResponse));
            }
        },
        [this](const std::string &errorMessage)
        {
            if (_speechErrorCallback)
            {
                _speechErrorCallback(errorMessage);
            }
        },
        [this]()
        {
            if (_speechDisconnectedCallback)
            {
                _speechDisconnectedCallback();
            }
        },
        [this]()
        {
            if (_speechStartedCallback)
            {
                _speechStartedCallback();
            }
        });
}

deepgram::speak::SpeakWebsocketClient::~SpeakWebsocketClient()
{

}

bool deepgram::speak::SpeakWebsocketClient::connect(const LiveSpeakConfig &config)
{
    if(_speakWebsocketClientImpl) {
        return _speakWebsocketClientImpl->connect(config);
    }
    return false;
}

void deepgram::speak::SpeakWebsocketClient::close()
{
    // Always delegate: SpeakWebsocketClientImpl::close() safely no-ops the
    // network teardown when already disconnected, but still must run so the
    // speech-timeout monitor thread gets joined.
    if (_speakWebsocketClientImpl) {
        _speakWebsocketClientImpl->close();
    } else {
        spdlog::error("can't close, SpeakWebsocketClientImpl is not initialized");
    }
}

void deepgram::speak::SpeakWebsocketClient::startReceiving()
{
    // Message/audio callbacks are already wired up (see constructor). The only
    // thing that still needs an explicit "connected" starting point is the
    // end-of-speech idle-timeout monitor, since it polls _wsTransport->isOpen().
    if (_speakWebsocketClientImpl && _speakWebsocketClientImpl->isConnected()) {
        _speakWebsocketClientImpl->startSpeechTimeoutMonitor(
            [this]() {
                if (_speechEndedCallback) {
                    _speechEndedCallback();
                }
            });
    } else {
        spdlog::error("can't start receiving, SpeakWebsocketClientImpl is not initialized");
    }
}

bool deepgram::speak::SpeakWebsocketClient::speak(const std::string &text)
{
    if (_speakWebsocketClientImpl && _speakWebsocketClientImpl->isConnected()) {
        nlohmann::json speakMessage = {
            {"type", "Speak"},
            {"text", text}
        };
        return _speakWebsocketClientImpl->sendPayload(speakMessage.dump());
    } else {
        spdlog::error("can't send text, SpeakWebsocketClientImpl is not initialized");
        return false;
    }
}

bool deepgram::speak::SpeakWebsocketClient::sendFlushMessage()
{
    if(_speakWebsocketClientImpl && _speakWebsocketClientImpl->isConnected()) {
        return _speakWebsocketClientImpl->sendPayload(speak::control::FLUSH);
    } else {
        spdlog::error("can't send flush message, SpeakWebsocketClientImpl is not initialized");
        return false;
    }
}

bool deepgram::speak::SpeakWebsocketClient::sendClearMessage()
{
    if (_speakWebsocketClientImpl && _speakWebsocketClientImpl->isConnected()) {
        return _speakWebsocketClientImpl->sendPayload(speak::control::CLEAR);
    } else {
        spdlog::error("can't send clear message, SpeakWebsocketClientImpl is not initialized");
        return false;
    }
}

bool deepgram::speak::SpeakWebsocketClient::sendCloseMessage()
{
    if (_speakWebsocketClientImpl && _speakWebsocketClientImpl->isConnected()) {
        return _speakWebsocketClientImpl->sendPayload(speak::control::CLOSE);
    } else {
        spdlog::error("can't send close message, SpeakWebsocketClientImpl is not initialized");
        return false;
    }
}

void deepgram::speak::SpeakWebsocketClient::setSpeechResultCallback(SpeechResultCallback callback)
{
    _speechResultCallback = std::move(callback);
}

void deepgram::speak::SpeakWebsocketClient::setSpeechControlResponseCallback(SpeechControlResponseCallback callback)
{
    _speechControlResponseCallback = std::move(callback);
}

void deepgram::speak::SpeakWebsocketClient::setSpeechCloseFrameCallback(SpeechCloseFrameCallback callback)
{
    _speechCloseFrameCallback = std::move(callback);
}

void deepgram::speak::SpeakWebsocketClient::setSpeechMetadataResponseCallback(SpeechMetadataResponseCallback callback)
{
    _speechMetadataResponseCallback = std::move(callback);
}

void deepgram::speak::SpeakWebsocketClient::setSpeechErrorCallback(SpeechErrorCallback callback)
{
    _speechErrorCallback = std::move(callback);
}

void deepgram::speak::SpeakWebsocketClient::setSpeechDisconnectedCallback(SpeechDisconnectedCallback callback)
{
    _speechDisconnectedCallback = std::move(callback);
}

void deepgram::speak::SpeakWebsocketClient::setSpeechStartedCallback(SpeechStartedCallback callback)
{
    _speechStartedCallback = std::move(callback);
}

void deepgram::speak::SpeakWebsocketClient::setSpeechEndedCallback(SpeechEndedCallback callback)
{
    _speechEndedCallback = std::move(callback);
}

void deepgram::speak::SpeakWebsocketClient::setSpeechReceptionTimeout(int timeoutMs)
{
    if (_speakWebsocketClientImpl) {
        _speakWebsocketClientImpl->setSpeechReceptionTimeout(timeoutMs);
    }
}
