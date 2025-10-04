#include "listen-flux.hpp"
#include "impl/listen-flux-impl-boost.hpp"
#include <spdlog/spdlog.h>

deepgram::listen::flux::ListenFluxClient::ListenFluxClient(const std::string& apiKey) : _fluxClientImpl(
    std::make_unique<ListenFluxClientImpl>("api.deepgram.com", apiKey, "443")
)
{
}

deepgram::listen::flux::ListenFluxClient::~ListenFluxClient()
{
}

bool deepgram::listen::flux::ListenFluxClient::connect(const FluxQueryParams& params)
{
    if (!_fluxClientImpl) {
        spdlog::error("cannot connect, ListenFluxClientImpl is not initialized.");
        return false;
    }
    return _fluxClientImpl->connect(params);
}

void deepgram::listen::flux::ListenFluxClient::startReceiving()
{
    if (!_fluxClientImpl) {
        spdlog::error("cannot start receiving, ListenFluxClientImpl is not initialized.");
        return;
    }

    std::function<void(const std::string&)> onDataReception = [this](const std::string& message) {
        try {
            auto jsonPayload = nlohmann::json::parse(message);
            std::string type = jsonPayload.value("type", "");
            if (type == event::type::TURN_INFO) {
                if (this->_onTurnInfoCallback) {
                    auto turnInfo = event::TurnInfo::fromJson(jsonPayload);
                    this->_onTurnInfoCallback(turnInfo);
                }
            }
            else if (type == event::type::CONNECTED) {
                if (this->_onConnectedCallback) {
                    auto connected = event::Connected::fromJson(jsonPayload);
                    this->_onConnectedCallback(connected);
                }
            }
            else if (type == event::type::FATAL_ERROR) {
                if (this->_onFatalErrorCallback) {
                    auto fatalError = event::FatalError::fromJson(jsonPayload);
                    this->_onFatalErrorCallback(fatalError);
                }
            }
            else {
                spdlog::warn("Received unknown message type: {}", type);
            }
        }
        catch (const std::exception& e) {
            spdlog::error("Error parsing message: {}, error: {}", message, e.what());
        }
        };
    _fluxClientImpl->startReceiving(onDataReception);
}

void deepgram::listen::flux::ListenFluxClient::stopReceiving()
{
    if (!_fluxClientImpl) {
        spdlog::error("cannot stop receiving, ListenFluxClientImpl is not initialized.");
        return;
    }
    _fluxClientImpl->stopReceiving();
}

bool deepgram::listen::flux::ListenFluxClient::streamAudio(const std::vector<uint8_t>& audioData, int chunkSize)
{
    if (!_fluxClientImpl) {
        spdlog::error("cannot stream audio, ListenFluxClientImpl is not initialized.");
        return false;
    }
    return _fluxClientImpl->streamAudio(audioData, chunkSize);
}

void deepgram::listen::flux::ListenFluxClient::sendCloseStream()
{
    if (!_fluxClientImpl) {
        spdlog::error("cannot close stream, ListenFluxClientImpl is not initialized.");
        return;
    }
    _fluxClientImpl->sendCloseStream();
}

void deepgram::listen::flux::ListenFluxClient::setOnTurnInfoCallback(OnTurnInfoCallback callback)
{
    _onTurnInfoCallback = callback;
}

void deepgram::listen::flux::ListenFluxClient::setOnConnectedCallback(OnConnectedCallback callback)
{
    _onConnectedCallback = callback;
}

void deepgram::listen::flux::ListenFluxClient::setOnFatalErrorCallback(OnFatalErrorCallback callback)
{
    _onFatalErrorCallback = callback;
}

