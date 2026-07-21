#pragma once

#include <deepgrampp_lib_export.h>
#include "http_transport.hpp"

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
            HttpResponse send(const HttpRequest &request) override;
        };

    } // namespace transport
} // namespace deepgram
