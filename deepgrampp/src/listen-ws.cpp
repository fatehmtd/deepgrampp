
#include "listen-ws.hpp"
#include "impl/listen-ws-impl-lws.hpp"
#include <spdlog/spdlog.h>

using namespace deepgram::listen;

ListenWebsocketClient::ListenWebsocketClient(const std::string &apiKey,
                                              std::shared_ptr<transport::IWebSocketTransport> wsTransport)
{
    websocketClientImpl_ = std::make_unique<ListenWebsocketClientImpl>("api.deepgram.com", apiKey, std::move(wsTransport));
    // Wired up now, before connect() is ever called, so no messages are missed.
    websocketClientImpl_->setHandlers(
        [this](const std::string &message)
        { handleResponse(message); },
        [this](const std::string &error)
        { onError_(error); });
}

ListenWebsocketClient::~ListenWebsocketClient()
{
    close();
}

bool ListenWebsocketClient::connect(const LiveTranscriptionOptions &options)
{
    if (!websocketClientImpl_) {
        spdlog::error("can't connect, websocketClientImpl_ is not initialized");
        return false;
    }
    return websocketClientImpl_->connect(options);
}

void ListenWebsocketClient::startReceiving()
{
    // No-op: message delivery starts automatically once connect() succeeds.
    // Kept for source compatibility.
}

void ListenWebsocketClient::startKeepalive()
{
    if (!websocketClientImpl_) {
        spdlog::error("can't start keepalive, websocketClientImpl_ is not initialized");
        return;
    }
    websocketClientImpl_->startKeepalive();
}

bool ListenWebsocketClient::streamAudio(const std::vector<uint8_t> &audioData, int chunkSize)
{
    if (!websocketClientImpl_) {
        spdlog::error("can't stream audio file, websocketClientImpl_ is not initialized");
        return false;
    }
    return websocketClientImpl_->streamAudio(audioData, chunkSize);
}

bool deepgram::listen::ListenWebsocketClient::sendFinalizeMessage()
{
    if (!websocketClientImpl_) {
        spdlog::error("can't send finalize message, websocketClientImpl_ is not initialized");
        return false;
    }
    return websocketClientImpl_->sendFinalizeMessage();
}

bool ListenWebsocketClient::sendAudioChunk(const uint8_t *data, size_t size)
{
    if (!websocketClientImpl_) {
        spdlog::error("can't send audio chunk, websocketClientImpl_ is not initialized");
        return false;
    }
    return websocketClientImpl_->sendAudioChunk(data, size);
}

void ListenWebsocketClient::sendCloseStream()
{
    if (!websocketClientImpl_) {
        spdlog::error("can't send close stream, websocketClientImpl_ is not initialized");
        return;
    }
    websocketClientImpl_->sendCloseStream();
}

void ListenWebsocketClient::handleResponse(const std::string &message)
{
    try
    {
        nlohmann::json json = nlohmann::json::parse(message);

        deepgram::listen::TranscriptionResult transcriptionResult = deepgram::listen::TranscriptionResult::fromJson(json);

        if (transcriptionResult.type == result::RESULTS)
        {
            if (transcriptionResult.isFinal || transcriptionResult.speech_final)
            {
                onFinalTranscription_(transcriptionResult);
            }
            else
            {
                onPartialTranscription_(transcriptionResult);
            }
        }
        else if (transcriptionResult.type == result::METADATA)
        {
            onMetadata_(json);
        }
        else if (transcriptionResult.type == result::UTTERANCE_END)
        {
            deepgram::listen::UtteranceEnd utteranceEnd = deepgram::listen::UtteranceEnd::fromJson(json);
            onUtteranceEnd_(utteranceEnd);
        }
        else if (transcriptionResult.type == result::SPEECH_STARTED)
        {
            deepgram::listen::SpeechStarted speechStarted = deepgram::listen::SpeechStarted::fromJson(json);
            onSpeechStarted_(speechStarted);
        }
        else
        {
            std::cout << "Received unknown message type: " << transcriptionResult.type << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error handling response: " << e.what() << std::endl;
        std::cout << "Raw message: " << message << std::endl;
    }
}

void ListenWebsocketClient::stopReceiving()
{
    // No-op: message delivery stops automatically when close() is called.
    // Kept for source compatibility.
}

void ListenWebsocketClient::close()
{
    if (!websocketClientImpl_) {
        spdlog::error("can't close, websocketClientImpl_ is not initialized");
        return;
    }
    websocketClientImpl_->close();
}

void ListenWebsocketClient::setOnPartialTranscription(PartialTranscriptionCallback cb)
{
    onPartialTranscription_ = std::move(cb);
}

void ListenWebsocketClient::setOnFinalTranscription(FinalTranscriptionCallback cb)
{
    onFinalTranscription_ = std::move(cb);
}

void ListenWebsocketClient::setOnMetadata(MetadataCallback cb)
{
    onMetadata_ = std::move(cb);
}

void ListenWebsocketClient::setOnError(ErrorCallback cb)
{
    onError_ = std::move(cb);
}

void ListenWebsocketClient::setOnSpeechStarted(SpeechStartedCallback cb)
{
    onSpeechStarted_ = std::move(cb);
}

void deepgram::listen::ListenWebsocketClient::setUtteranceEndCallback(UtteranceEndCallback cb)
{
    onUtteranceEnd_ = std::move(cb);
}
