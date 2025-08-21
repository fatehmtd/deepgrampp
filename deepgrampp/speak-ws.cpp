#include "speak-ws-impl-boost.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "speak-ws.hpp"

deepgram::speak::SpeakWebsocketClient::SpeakWebsocketClient(const std::string &host, const std::string &apiKey, const std::string &port)
{
    _speakWebsocketClientImpl = std::make_unique<SpeakWebsocketClientImpl>(host, apiKey, port);
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
    if (_speakWebsocketClientImpl && _speakWebsocketClientImpl->isConnected()) {
        _speakWebsocketClientImpl->close();
    } else {
        spdlog::error("can't close, SpeakWebsocketClientImpl is not initialized");
    }
}

void deepgram::speak::SpeakWebsocketClient::startReceiving()
{
    if (_speakWebsocketClientImpl && _speakWebsocketClientImpl->isConnected()) {
        _speakWebsocketClientImpl->startReceiving(
            [this](const char *data, int size) {
                if (_speechResultCallback) {
                    _speechResultCallback(data, size);
                }
            },
            [this](const std::string &message) {
                nlohmann::json jsonResponse = nlohmann::json::parse(message);
                if(jsonResponse.contains("type")) {
                    if(_speechControlResponseCallback) {
                        if(jsonResponse["type"] == "Metadata") {
                            _speechMetadataResponseCallback(MetadataResponse::fromJson(jsonResponse));
                        } else {
                            _speechControlResponseCallback(SpeakControlResponse::fromJson(jsonResponse));
                        }
                    }
                } else {
                    if (_speechCloseFrameCallback) {
                        _speechCloseFrameCallback(SpeakCloseFrame::fromJson(jsonResponse));
                    }
                }
            }
        );
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
