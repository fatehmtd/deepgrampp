#pragma once

#include <deepgrampp_lib_export.h>
#include "speak.hpp"
#include "transport/http_transport.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace deepgram
{
    namespace speak
    {
        class SpeakRestClientImpl;

        /**
         * @brief Result of a batch text-to-speech request.
         *
         * `audio`/`contentType`/`requestId`/`modelName` are only meaningful
         * when `success` is true.
         */
        struct SpeakRestResult
        {
            bool success = false;
            long statusCode = 0;
            std::string errorMessage;
            std::vector<uint8_t> audio;
            std::string contentType;
            std::optional<std::string> requestId;
            std::optional<std::string> modelName;

            explicit operator bool() const { return success; }
        };

        /**
         * @brief REST client for Deepgram's batch text-to-speech API
         * (POST /v1/speak). Unlike SpeakWebsocketClient, this issues a single
         * blocking HTTP request per call and returns the full audio buffer
         * directly, with no persistent connection or callbacks involved.
         */
        class DEEPGRAMPP_EXPORT SpeakRestClient
        {
        public:
            /**
             * @param apiKey Your Deepgram API key for authentication.
             * @param httpTransport Optional HTTP transport to use instead of the
             *        default libcurl-based transport (deepgram::transport::CurlHttpTransport).
             *        Useful for tests or to plug in a different networking stack.
             * @param caFilePath Path to a PEM-encoded CA bundle passed to the default
             *        transport when `httpTransport` is null. Needed on platforms where
             *        the bundled mbedTLS backend has no visibility into the OS trust
             *        store (e.g. Android); leave empty elsewhere. Ignored if
             *        `httpTransport` is provided.
             */
            SpeakRestClient(const std::string &apiKey,
                             std::shared_ptr<transport::IHttpTransport> httpTransport = nullptr,
                             const std::string &caFilePath = {});
            ~SpeakRestClient();

            /**
             * @brief Synthesizes speech for the given text.
             */
            SpeakRestResult speak(const std::string &text, const LiveSpeakConfig &config = {});

        private:
            std::unique_ptr<SpeakRestClientImpl> impl_;
        };
    }
}
