
#include "listen-ws.hpp"
#include "listen-ws-impl-boost.hpp"
#include <spdlog/spdlog.h>

using namespace deepgram::listen;

using namespace boost;
using namespace boost::beast;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using Websocket = websocket::stream<ssl::stream<tcp::socket>>;

ListenWebsocketClient::ListenWebsocketClient(const std::string &host,
                                             const std::string &apiKey,
                                             const std::string &port)
{
    websocketClientImpl_ = new ListenWebsocketClientImpl(host, apiKey, port);
}

ListenWebsocketClient::~ListenWebsocketClient()
{
    close();
    if (websocketClientImpl_ != nullptr)
    {
        delete websocketClientImpl_;
        websocketClientImpl_ = nullptr;
    }
}

bool ListenWebsocketClient::connect(const LiveTranscriptionOptions &options)
{
    return websocketClientImpl_->connect(options);
}

void ListenWebsocketClient::startReceiving()
{
    websocketClientImpl_->startReceiving([this](const std::string& message) {
        handleResponse(message);
    });
}

void ListenWebsocketClient::startKeepalive()
{
    websocketClientImpl_->startKeepalive();
}

bool ListenWebsocketClient::streamAudioFile(const std::vector<uint8_t> &audioData)
{
    return websocketClientImpl_->streamAudioFile(audioData, 4000);
}

bool deepgram::listen::ListenWebsocketClient::sendFinalizeMessage()
{
    return websocketClientImpl_->sendFinalizeMessage();
}

bool ListenWebsocketClient::sendAudioChunk(const uint8_t *data, size_t size)
{
    return websocketClientImpl_->sendAudioChunk(data, size);
}

void ListenWebsocketClient::sendCloseStream()
{
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
    websocketClientImpl_->stopReceiving();
}

void ListenWebsocketClient::close()
{
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
