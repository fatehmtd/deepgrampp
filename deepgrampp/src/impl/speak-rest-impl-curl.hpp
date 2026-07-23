#pragma once

#include "../../include/deepgrampp/speak-rest.hpp"
#include "../../include/deepgrampp/transport/curl_http_transport.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace deepgram
{
    namespace speak
    {
        class SpeakRestClientImpl
        {
        public:
            SpeakRestClientImpl(const std::string &host,
                                 const std::string &apiKey,
                                 std::shared_ptr<transport::IHttpTransport> httpTransport,
                                 const std::string &caFilePath = {})
                : _host(host), _apiKey(apiKey),
                  _httpTransport(httpTransport ? std::move(httpTransport)
                                                : std::make_shared<transport::CurlHttpTransport>(caFilePath))
            {
            }

            SpeakRestResult speak(const std::string &text, const LiveSpeakConfig &config)
            {
                transport::HttpRequest request;
                request.method = transport::HttpMethod::Post;
                request.url = "https://" + _host + config.toQueryString();
                request.headers["Authorization"] = "Token " + _apiKey;
                request.content_type = "application/json";
                request.body = nlohmann::json{{"text", text}}.dump();

                SpeakRestResult result;
                transport::HttpResponse response;
                try
                {
                    response = _httpTransport->send(request);
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Speak request failed: {}", e.what());
                    result.success = false;
                    result.errorMessage = e.what();
                    return result;
                }

                result.statusCode = response.status_code;

                if (response.status_code < 200 || response.status_code >= 300)
                {
                    std::string bodyText(response.body.begin(), response.body.end());
                    spdlog::error("Speak request returned HTTP {}: {}", response.status_code, bodyText);
                    result.success = false;
                    result.errorMessage = bodyText;
                    return result;
                }

                result.audio = response.body;
                if (response.headers.count("content-type"))
                {
                    result.contentType = response.headers.at("content-type");
                }
                if (response.headers.count("dg-request-id"))
                {
                    result.requestId = response.headers.at("dg-request-id");
                }
                if (response.headers.count("dg-model-name"))
                {
                    result.modelName = response.headers.at("dg-model-name");
                }
                result.success = true;
                return result;
            }

        private:
            std::string _host;
            std::string _apiKey;
            std::shared_ptr<transport::IHttpTransport> _httpTransport;
        };
    }
}
