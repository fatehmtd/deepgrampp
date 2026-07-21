#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace deepgram
{
    namespace transport
    {
        enum class HttpMethod
        {
            Get,
            Post,
            Put,
            Delete
        };

        struct HttpRequest
        {
            HttpMethod method{HttpMethod::Get};
            std::string url;
            std::map<std::string, std::string> headers;
            std::string content_type;
            std::string body;                      // JSON or text body
            std::vector<std::uint8_t> binary_body; // raw audio bytes
            long timeout_ms{0};
        };

        struct HttpResponse
        {
            long status_code{0};
            std::map<std::string, std::string> headers;
            std::vector<std::uint8_t> body;
        };

        /**
         * Synchronous HTTP transport interface. Not required to be thread-safe.
         */
        class IHttpTransport
        {
        public:
            virtual ~IHttpTransport() = default;

            /**
             * Throws std::runtime_error on I/O or TLS errors.
             * HTTP >= 400 is not thrown - callers check status_code.
             */
            virtual HttpResponse send(const HttpRequest &request) = 0;
        };

    } // namespace transport
} // namespace deepgram
