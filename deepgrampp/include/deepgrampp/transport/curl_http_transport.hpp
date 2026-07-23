#pragma once

#include <deepgrampp_lib_export.h>
#include "http_transport.hpp"

#include <string>

namespace deepgram
{
    namespace transport
    {
        /**
         * HTTP transport backed by libcurl. Intended for Deepgram's REST endpoints
         * (pre-recorded transcription, one-shot text-to-speech); not used by the
         * WebSocket streaming clients.
         */
        class DEEPGRAMPP_EXPORT CurlHttpTransport final : public IHttpTransport
        {
        public:
            /**
             * @param caFilePath Path to a PEM-encoded CA bundle to use for verifying
             *        server certificates. Needed on platforms where the bundled
             *        mbedTLS backend has no visibility into the OS trust store (e.g.
             *        Android); leave empty elsewhere to keep using curl's platform
             *        default.
             */
            explicit CurlHttpTransport(std::string caFilePath = {});

            HttpResponse send(const HttpRequest &request) override;

        private:
            std::string _caFilePath;
        };

    } // namespace transport
} // namespace deepgram
