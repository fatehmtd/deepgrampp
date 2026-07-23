#include "speak-rest.hpp"
#include "impl/speak-rest-impl-curl.hpp"
#include <spdlog/spdlog.h>

using namespace deepgram::speak;

SpeakRestClient::SpeakRestClient(const std::string &apiKey,
                                  std::shared_ptr<transport::IHttpTransport> httpTransport,
                                  const std::string &caFilePath)
{
    impl_ = std::make_unique<SpeakRestClientImpl>("api.deepgram.com", apiKey, std::move(httpTransport), caFilePath);
}

SpeakRestClient::~SpeakRestClient() = default;

SpeakRestResult SpeakRestClient::speak(const std::string &text, const LiveSpeakConfig &config)
{
    if (!impl_)
    {
        spdlog::error("can't speak, SpeakRestClientImpl is not initialized");
        return SpeakRestResult{};
    }
    return impl_->speak(text, config);
}
