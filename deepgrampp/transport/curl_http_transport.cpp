#include <deepgrampp/transport/curl_http_transport.hpp>

#include <curl/curl.h>

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

namespace deepgram
{
    namespace transport
    {
        namespace
        {
            struct CurlResponseContext
            {
                HttpResponse response;
            };

            std::string toLowerCopy(const std::string &value)
            {
                std::string out = value;
                std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c)
                                { return static_cast<char>(std::tolower(c)); });
                return out;
            }

            std::string trimCopy(const std::string &value)
            {
                std::size_t start = 0;
                while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0)
                {
                    ++start;
                }
                std::size_t end = value.size();
                while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0)
                {
                    --end;
                }
                return value.substr(start, end - start);
            }

            size_t writeBodyCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
            {
                const auto bytes = size * nmemb;
                auto *ctx = static_cast<CurlResponseContext *>(userdata);
                auto *body = &ctx->response.body;
                const auto *begin = reinterpret_cast<const std::uint8_t *>(ptr);
                body->insert(body->end(), begin, begin + bytes);
                return bytes;
            }

            size_t writeHeaderCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
            {
                const auto bytes = size * nmemb;
                std::string line(ptr, bytes);
                auto *ctx = static_cast<CurlResponseContext *>(userdata);

                const auto colon = line.find(':');
                if (colon == std::string::npos)
                {
                    return bytes;
                }

                const std::string key = toLowerCopy(trimCopy(line.substr(0, colon)));
                const std::string value = trimCopy(line.substr(colon + 1));
                if (!key.empty())
                {
                    ctx->response.headers[key] = value;
                }
                return bytes;
            }

        } // namespace

        HttpResponse CurlHttpTransport::send(const HttpRequest &request)
        {
            static const int curlGlobalInitResult = []
            { return curl_global_init(CURL_GLOBAL_DEFAULT); }();

            if (curlGlobalInitResult != CURLE_OK)
            {
                throw std::runtime_error("[deepgrampp] curl_global_init failed");
            }

            CURL *curl = curl_easy_init();
            if (curl == nullptr)
            {
                throw std::runtime_error("[deepgrampp] curl_easy_init failed");
            }

            CurlResponseContext ctx;

            curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBodyCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaderCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ctx);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            if (request.timeout_ms > 0)
            {
                curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, request.timeout_ms);
            }

            switch (request.method)
            {
            case HttpMethod::Get:
                break;

            case HttpMethod::Post:
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                if (!request.binary_body.empty())
                {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.binary_body.data());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(request.binary_body.size()));
                }
                else if (!request.body.empty())
                {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(request.body.size()));
                }
                break;

            case HttpMethod::Put:
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
                if (!request.body.empty())
                {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(request.body.size()));
                }
                break;

            case HttpMethod::Delete:
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
                break;
            }

            curl_slist *headerList = nullptr;
            for (const auto &[key, value] : request.headers)
            {
                const std::string item = key + ": " + value;
                headerList = curl_slist_append(headerList, item.c_str());
            }
            if (!request.content_type.empty())
            {
                const std::string ct = "Content-Type: " + request.content_type;
                headerList = curl_slist_append(headerList, ct.c_str());
            }
            if (headerList != nullptr)
            {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
            }

            const CURLcode rc = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ctx.response.status_code);

            if (headerList != nullptr)
            {
                curl_slist_free_all(headerList);
            }
            curl_easy_cleanup(curl);

            if (rc != CURLE_OK)
            {
                throw std::runtime_error(std::string("[deepgrampp] curl perform failed: ") + curl_easy_strerror(rc));
            }

            return ctx.response;
        }

    } // namespace transport
} // namespace deepgram
