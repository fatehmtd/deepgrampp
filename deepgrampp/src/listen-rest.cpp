#include "listen-rest.hpp"
#include "impl/listen-rest-impl-curl.hpp"
#include <spdlog/spdlog.h>

using namespace deepgram::listen;

ListenRestClient::ListenRestClient(const std::string &apiKey,
                                    std::shared_ptr<transport::IHttpTransport> httpTransport)
{
    impl_ = std::make_unique<ListenRestClientImpl>("api.deepgram.com", apiKey, std::move(httpTransport));
}

ListenRestClient::~ListenRestClient() = default;

PrerecordedTranscriptionResult ListenRestClient::transcribeUrl(const std::string &audioUrl,
                                                                 const LiveTranscriptionOptions &options)
{
    if (!impl_)
    {
        spdlog::error("can't transcribe, ListenRestClientImpl is not initialized");
        return PrerecordedTranscriptionResult{};
    }
    return impl_->transcribeUrl(audioUrl, options);
}

PrerecordedTranscriptionResult ListenRestClient::transcribeBuffer(const std::vector<uint8_t> &audioData,
                                                                    const std::string &contentType,
                                                                    const LiveTranscriptionOptions &options)
{
    if (!impl_)
    {
        spdlog::error("can't transcribe, ListenRestClientImpl is not initialized");
        return PrerecordedTranscriptionResult{};
    }
    return impl_->transcribeBuffer(audioData, contentType, options);
}

PrerecordedTranscriptionResult ListenRestClient::transcribeFile(const std::string &filePath,
                                                                  const std::string &contentType,
                                                                  const LiveTranscriptionOptions &options)
{
    if (!impl_)
    {
        spdlog::error("can't transcribe, ListenRestClientImpl is not initialized");
        return PrerecordedTranscriptionResult{};
    }
    return impl_->transcribeFile(filePath, contentType, options);
}
