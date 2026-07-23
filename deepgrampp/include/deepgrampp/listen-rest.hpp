#pragma once

#include <deepgrampp_lib_export.h>
#include "listen.hpp"
#include "transport/http_transport.hpp"

#include <memory>
#include <string>
#include <vector>

namespace deepgram
{
    namespace listen
    {
        class ListenRestClientImpl;

        /**
         * @brief Result of a prerecorded/batch transcription request.
         *
         * `response` is only meaningful when `success` is true.
         */
        struct PrerecordedTranscriptionResult
        {
            bool success = false;
            long statusCode = 0;
            std::string errorMessage;
            PrerecordedTranscriptionResponse response;

            explicit operator bool() const { return success; }
        };

        /**
         * @brief REST client for Deepgram's prerecorded/batch transcription API
         * (POST /v1/listen). Unlike ListenWebsocketClient, this issues a single
         * blocking HTTP request per call and returns the full result directly,
         * with no persistent connection or callbacks involved.
         */
        class DEEPGRAMPP_EXPORT ListenRestClient
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
            ListenRestClient(const std::string &apiKey,
                              std::shared_ptr<transport::IHttpTransport> httpTransport = nullptr,
                              const std::string &caFilePath = {});
            ~ListenRestClient();

            /**
             * @brief Transcribes audio hosted at a remote URL.
             */
            PrerecordedTranscriptionResult transcribeUrl(const std::string &audioUrl,
                                                           const LiveTranscriptionOptions &options = {});

            /**
             * @brief Transcribes audio already available in memory.
             *
             * @param audioData Raw audio bytes.
             * @param contentType MIME type of the audio, e.g. "audio/wav", "audio/L16".
             *
             * @note `options.sampleRate`/`options.encoding`/`options.channels` are sent as
             *       query params and describe headerless raw PCM. If `audioData` already
             *       has its own container (a real .wav/.mp3/.flac file's bytes), those
             *       params will make Deepgram misinterpret the container as raw PCM and
             *       silently return a garbage/empty transcript -- only rely on the defaults
             *       here when `audioData` is genuinely headerless raw audio (as it is for
             *       the LINEAR_16 samples used elsewhere in this SDK).
             */
            PrerecordedTranscriptionResult transcribeBuffer(const std::vector<uint8_t> &audioData,
                                                              const std::string &contentType,
                                                              const LiveTranscriptionOptions &options = {});

            /**
             * @brief Convenience overload that reads the audio file off disk and
             * delegates to transcribeBuffer(). A file that can't be opened/read is
             * reported via `success = false` + `errorMessage`, never as a thrown
             * exception.
             *
             * @param filePath Path to a local audio file.
             * @param contentType MIME type of the audio, e.g. "audio/wav", "audio/L16".
             *
             * @note See the `transcribeBuffer()` note above regarding raw PCM vs.
             *       containerized files (.wav/.mp3/.flac) -- it applies here too.
             */
            PrerecordedTranscriptionResult transcribeFile(const std::string &filePath,
                                                            const std::string &contentType,
                                                            const LiveTranscriptionOptions &options = {});

        private:
            std::unique_ptr<ListenRestClientImpl> impl_;
        };
    }
}
