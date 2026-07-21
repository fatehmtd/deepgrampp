#pragma once

#include "../../include/deepgrampp/listen-rest.hpp"
#include "../../include/deepgrampp/transport/curl_http_transport.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

namespace deepgram
{
    namespace listen
    {
        class ListenRestClientImpl
        {
        public:
            ListenRestClientImpl(const std::string &host,
                                  const std::string &apiKey,
                                  std::shared_ptr<transport::IHttpTransport> httpTransport)
                : _host(host), _apiKey(apiKey),
                  _httpTransport(httpTransport ? std::move(httpTransport)
                                                : std::make_shared<transport::CurlHttpTransport>())
            {
            }

            PrerecordedTranscriptionResult transcribeUrl(const std::string &audioUrl,
                                                           const LiveTranscriptionOptions &options)
            {
                transport::HttpRequest request;
                request.method = transport::HttpMethod::Post;
                // A remote URL always fetches a real file with its own container (WAV, MP3,
                // etc.), so sample_rate/encoding/channels (which describe headerless raw PCM)
                // must be omitted -- otherwise Deepgram misinterprets the container bytes as
                // raw PCM and silently returns an empty transcript with a bogus duration.
                request.url = "https://" + _host + options.toQueryString("/v1/listen", false);
                request.headers["Authorization"] = "Token " + _apiKey;
                request.content_type = "application/json";
                request.body = nlohmann::json{{"url", audioUrl}}.dump();
                return doSend(request);
            }

            PrerecordedTranscriptionResult transcribeBuffer(const std::vector<uint8_t> &audioData,
                                                              const std::string &contentType,
                                                              const LiveTranscriptionOptions &options)
            {
                transport::HttpRequest request;
                request.method = transport::HttpMethod::Post;
                request.url = "https://" + _host + options.toQueryString();
                request.headers["Authorization"] = "Token " + _apiKey;
                request.content_type = contentType;
                request.binary_body = audioData;
                return doSend(request);
            }

            PrerecordedTranscriptionResult transcribeFile(const std::string &filePath,
                                                            const std::string &contentType,
                                                            const LiveTranscriptionOptions &options)
            {
                try
                {
                    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
                    if (!file)
                    {
                        throw std::runtime_error("Failed to open audio file: " + filePath);
                    }
                    std::streamsize size = file.tellg();
                    file.seekg(0, std::ios::beg);
                    std::vector<uint8_t> buffer(size);
                    if (size > 0 && !file.read(reinterpret_cast<char *>(buffer.data()), size))
                    {
                        throw std::runtime_error("Failed to read audio file: " + filePath);
                    }
                    return transcribeBuffer(buffer, contentType, options);
                }
                catch (const std::exception &e)
                {
                    spdlog::error("transcribeFile error: {}", e.what());
                    PrerecordedTranscriptionResult result;
                    result.success = false;
                    result.errorMessage = e.what();
                    return result;
                }
            }

        private:
            PrerecordedTranscriptionResult doSend(const transport::HttpRequest &request)
            {
                PrerecordedTranscriptionResult result;
                transport::HttpResponse response;
                try
                {
                    response = _httpTransport->send(request);
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Prerecorded transcription request failed: {}", e.what());
                    result.success = false;
                    result.errorMessage = e.what();
                    return result;
                }

                result.statusCode = response.status_code;
                std::string bodyText(response.body.begin(), response.body.end());

                if (response.status_code < 200 || response.status_code >= 300)
                {
                    spdlog::error("Prerecorded transcription request returned HTTP {}: {}", response.status_code, bodyText);
                    result.success = false;
                    result.errorMessage = bodyText;
                    return result;
                }

                try
                {
                    nlohmann::json json = nlohmann::json::parse(bodyText);
                    result.response = PrerecordedTranscriptionResponse::fromJson(json);
                    result.success = true;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to parse prerecorded transcription response: {}", e.what());
                    result.success = false;
                    result.errorMessage = e.what();
                }
                return result;
            }

            std::string _host;
            std::string _apiKey;
            std::shared_ptr<transport::IHttpTransport> _httpTransport;
        };
    }
}
