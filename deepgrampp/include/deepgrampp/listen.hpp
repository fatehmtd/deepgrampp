#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <optional>
#include <nlohmann/json.hpp>

namespace deepgram
{
    namespace listen
    {
        namespace models
        {
            /**
             * NOVA 3
             *
             * @note NOVA 3 models are the latest generation of Deepgram's speech-to-text models.
             *       They are designed to provide high accuracy and performance for a wide range of applications.
             *       See https://developers.deepgram.com/docs/nova3 for more details.
             */
            namespace nova_3
            {
                constexpr const char *GENERAL = "nova-3-general";
                constexpr const char *MEDICAL = "nova-3-medical";
            }

            /**
             * NOVA 2
             *
             * @note NOVA 2 models are deprecated and will be removed in a future release.
             *       Use NOVA3 models instead.
             *       See https://developers.deepgram.com/docs/nova-2-deprecation for more details.
             *       @deprecated NOVA2 models are deprecated and will be removed in a future release.
             *       Use NOVA3 models instead.
             */
            namespace nova_2
            {
                // general: (Default) Optimized for everyday audio processing.
                constexpr const char *GENERAL = "nova-2-general";
                constexpr const char *MEETING = "nova-2-meeting";
                // Optimized for low-bandwidth audio phone calls.
                constexpr const char *PHONECALL = "nova-2-phonecall";
                constexpr const char *VOICEMAIL = "nova-2-voicemail";
                constexpr const char *FINANCE = "nova-2-finance";
                constexpr const char *CONVERSATIONALAI = "nova-2-conversationalai";
                constexpr const char *VIDEO = "nova-2-video";
                constexpr const char *DRIVETHRU = "nova-2-drivethru";
                constexpr const char *MEDICAL = "nova-2-medical";
                constexpr const char *AUTOMOTIVE = "nova-2-automotive";
                constexpr const char *ATC = "nova-2-atc";
            }

            /**
             * NOVA
             *
             * @note NOVA models are the original Deepgram speech-to-text models.
             *       They are still available for use, but are not recommended for new applications.
             *       See https://developers.deepgram.com/docs/nova-models for more details.
             *       @deprecated NOVA models are deprecated and will be removed in a future release.
             *       Use NOVA3 models instead.
             */
            namespace nova_1
            {
                // general: (Default) Optimized for everyday audio processing.
                constexpr const char *GENERAL = "nova-general";
                // Optimized for low-bandwidth audio phone calls.
                constexpr const char *PHONECALL = "nova-phonecall";
            }

            /**
             * BASE
             * @note BASE models are the original Deepgram speech-to-text models.
             *       They are still available for use, but are not recommended for new applications.
             *       See https://developers.deepgram.com/docs/base-models for more details.
             */
            namespace base
            {
                // general: (Default) Optimized for everyday audio processing.
                constexpr const char *GENERAL = "base-general";
                // Optimized for conference room settings, which include multiple speakers with a single microphone.
                constexpr const char *MEETING = "base-meeting";
                // Optimized for low-bandwidth audio phone calls.
                constexpr const char *PHONECALL = "base-phonecall";
                // voicemail: Optimized for low-bandwidth audio clips with a single speaker. Derived from the phonecall model.
                constexpr const char *VOICEMAIL = "base-voicemail";
                // Optimized for multiple speakers with varying audio quality, such as might be found on a typical earnings call. Vocabulary is heavily finance oriented.
                constexpr const char *FINANCE = "base-finance";
                // Optimized for use cases in which a human is talking to an automated bot, such as IVR, a voice assistant, or an automated kiosk.
                constexpr const char *CONVERSATIONALAI = "base-conversationalai";
                // Optimized for audio sourced from videos.
                constexpr const char *VIDEO = "base-video";
            }
        }

        namespace languages
        {
            // Nova-3 Model - Multilingual support (10 languages)
            // Note: Nova-3 multilingual uses language=multi parameter
            namespace nova_3
            {
                constexpr const char *ENGLISH = "en";
                constexpr const char *SPANISH = "es";
                constexpr const char *FRENCH = "fr";
                constexpr const char *GERMAN = "de";
                constexpr const char *HINDI = "hi";
                constexpr const char *RUSSIAN = "ru";
                constexpr const char *PORTUGUESE = "pt";
                constexpr const char *JAPANESE = "ja";
                constexpr const char *ITALIAN = "it";
                constexpr const char *DUTCH = "nl";

                // Special multilingual parameter for Nova-3
                constexpr const char *MULTILINGUAL = "multi";
            }

            // Nova-2 Model - 36 languages supported
            namespace nova_2
            {
                // Core languages
                constexpr const char *ENGLISH = "en";
                constexpr const char *SPANISH = "es";
                constexpr const char *FRENCH = "fr";
                constexpr const char *GERMAN = "de";
                constexpr const char *ITALIAN = "it";
                constexpr const char *PORTUGUESE = "pt";
                constexpr const char *RUSSIAN = "ru";
                constexpr const char *DUTCH = "nl";
                constexpr const char *JAPANESE = "ja";
                constexpr const char *KOREAN = "ko";
                constexpr const char *MANDARIN_CHINESE = "zh";
                constexpr const char *HINDI = "hi";

                // Additional languages (based on common language support patterns)
                constexpr const char *ARABIC = "ar";
                constexpr const char *TURKISH = "tr";
                constexpr const char *POLISH = "pl";
                constexpr const char *SWEDISH = "sv";
                constexpr const char *DANISH = "da";
                constexpr const char *NORWEGIAN = "no";
                constexpr const char *FINNISH = "fi";
                constexpr const char *CZECH = "cs";
                constexpr const char *HUNGARIAN = "hu";
                constexpr const char *ROMANIAN = "ro";
                constexpr const char *BULGARIAN = "bg";
                constexpr const char *CROATIAN = "hr";
                constexpr const char *SLOVAK = "sk";
                constexpr const char *SLOVENIAN = "sl";
                constexpr const char *LITHUANIAN = "lt";
                constexpr const char *LATVIAN = "lv";
                constexpr const char *ESTONIAN = "et";
                constexpr const char *UKRAINIAN = "uk";
                constexpr const char *GREEK = "el";
                constexpr const char *HEBREW = "he";
                constexpr const char *THAI = "th";
                constexpr const char *VIETNAMESE = "vi";
                constexpr const char *INDONESIAN = "id";
                constexpr const char *MALAY = "ms";

                // Special multilingual parameter for Nova-2
                constexpr const char *MULTILINGUAL = "multi";
            }

            // Nova (Nova-1) Model - Legacy model
            namespace nova_1
            {
                constexpr const char *ENGLISH = "en";
                constexpr const char *SPANISH = "es";
                constexpr const char *FRENCH = "fr";
                constexpr const char *GERMAN = "de";
                constexpr const char *ITALIAN = "it";
                constexpr const char *PORTUGUESE = "pt";
                constexpr const char *RUSSIAN = "ru";
                constexpr const char *DUTCH = "nl";
                constexpr const char *JAPANESE = "ja";
                constexpr const char *KOREAN = "ko";
                constexpr const char *MANDARIN_CHINESE = "zh";
                constexpr const char *HINDI = "hi";
            }

            // Enhanced Model
            namespace enhanced
            {
                constexpr const char *ENGLISH = "en";
                constexpr const char *SPANISH = "es";
                constexpr const char *FRENCH = "fr";
                constexpr const char *GERMAN = "de";
                constexpr const char *ITALIAN = "it";
                constexpr const char *PORTUGUESE = "pt";
                constexpr const char *RUSSIAN = "ru";
                constexpr const char *DUTCH = "nl";
                constexpr const char *JAPANESE = "ja";
                constexpr const char *KOREAN = "ko";
                constexpr const char *MANDARIN_CHINESE = "zh";
                constexpr const char *HINDI = "hi";
            }

            // Base Model
            namespace base
            {
                constexpr const char *ENGLISH = "en";
                constexpr const char *SPANISH = "es";
                constexpr const char *FRENCH = "fr";
                constexpr const char *GERMAN = "de";
                constexpr const char *ITALIAN = "it";
                constexpr const char *PORTUGUESE = "pt";
                constexpr const char *RUSSIAN = "ru";
                constexpr const char *DUTCH = "nl";
                constexpr const char *JAPANESE = "ja";
                constexpr const char *KOREAN = "ko";
                constexpr const char *MANDARIN_CHINESE = "zh";
                constexpr const char *HINDI = "hi";
            }

            // Whisper Model - OpenAI Whisper via Deepgram Cloud
            namespace whisper
            {
                // Whisper supports many languages - these are the most common ones
                constexpr const char *ENGLISH = "en";
                constexpr const char *SPANISH = "es";
                constexpr const char *FRENCH = "fr";
                constexpr const char *GERMAN = "de";
                constexpr const char *ITALIAN = "it";
                constexpr const char *PORTUGUESE = "pt";
                constexpr const char *RUSSIAN = "ru";
                constexpr const char *DUTCH = "nl";
                constexpr const char *JAPANESE = "ja";
                constexpr const char *KOREAN = "ko";
                constexpr const char *MANDARIN_CHINESE = "zh";
                constexpr const char *HINDI = "hi";
                constexpr const char *ARABIC = "ar";
                constexpr const char *TURKISH = "tr";
                constexpr const char *POLISH = "pl";
                constexpr const char *SWEDISH = "sv";
                constexpr const char *DANISH = "da";
                constexpr const char *NORWEGIAN = "no";
                constexpr const char *FINNISH = "fi";
                constexpr const char *CZECH = "cs";
                constexpr const char *HUNGARIAN = "hu";
                constexpr const char *ROMANIAN = "ro";
                constexpr const char *BULGARIAN = "bg";
                constexpr const char *CROATIAN = "hr";
                constexpr const char *SLOVAK = "sk";
                constexpr const char *SLOVENIAN = "sl";
                constexpr const char *LITHUANIAN = "lt";
                constexpr const char *LATVIAN = "lv";
                constexpr const char *ESTONIAN = "et";
                constexpr const char *UKRAINIAN = "uk";
                constexpr const char *GREEK = "el";
                constexpr const char *HEBREW = "he";
                constexpr const char *THAI = "th";
                constexpr const char *VIETNAMESE = "vi";
                constexpr const char *INDONESIAN = "id";
                constexpr const char *MALAY = "ms";
            }

            // English dialect variants (standardized to American spelling in output)
            namespace english_dialects
            {
                constexpr const char *US_ENGLISH = "en-US";
                constexpr const char *GB_ENGLISH = "en-GB";
                constexpr const char *AU_ENGLISH = "en-AU";
                constexpr const char *CA_ENGLISH = "en-CA";
                constexpr const char *IN_ENGLISH = "en-IN";
                constexpr const char *NZ_ENGLISH = "en-NZ";
                constexpr const char *ZA_ENGLISH = "en-ZA";
            }
        }

        /**
         * Audio Encoding Types
         * @note Supported audio encoding types for live transcription.
         *       See https://developers.deepgram.com/docs/sample-rate for more details.
         */
        namespace encoding
        {
            // 16-bit, little endian, signed PCM WAV data
            constexpr const char *LINEAR_16 = "linear16";
            // 32-bit, little endian, floating-point PCM WAV data
            constexpr const char *LINEAR_32 = "linear32";
            // Free Lossless Audio Codec (FLAC) encoded data
            constexpr const char *FLAC = "flac";
            // A-law encoded WAV data
            constexpr const char *ALAW = "alaw";
            // Mu-law encoded WAV data
            constexpr const char *MULAW = "mulaw";
            // Adaptive Multi-Rate (AMR) narrowband codec
            constexpr const char *AMR_NB = "amr-nb";
            // Adaptive Multi-Rate (AMR) wideband codec
            constexpr const char *AMR_WB = "amr-wb";
            // The Opus audio codec
            constexpr const char *OPUS = "opus";
            // The Opus audio codec encapsulated in the Ogg container format
            constexpr const char *OGG_OPUS = "ogg-opus";
            // An open-source, speech-specific audio codec
            constexpr const char *SPEEX = "speex";
            // G729 low-bandwidth (required for both raw and containerized audio)
            constexpr const char *G729 = "g729";
        }

        /**
         * LiveTranscriptionOptions
         * @note Options for configuring live transcription streams.
         * These options allow you to specify the model, language, encoding, and other parameters for the transcription service.
         */
        struct LiveTranscriptionOptions
        {
            // see https://developers.deepgram.com/docs/live-transcription-options
            std::string model = models::nova_3::GENERAL;            // Default model
            std::string language = languages::nova_3::MULTILINGUAL; // Default language
            std::optional<int> punctuate;                           // Default punctuation
            // note: see https://developers.deepgram.com/docs/sample-rate
            int sampleRate = 16000;                     // Default sample rate
            std::string encoding = encoding::LINEAR_16; // Default encoding
            int channels = 1;                           // Default number of channels
            std::optional<int> multichannel;            // Default multichannel
            std::optional<int> endpointing;             // Default endpointing in milliseconds
            std::optional<int> utteranceEndMs;          // Default utterance end timeout in milliseconds
            std::optional<bool> interimResults;         // Default interim results
            std::optional<int> vadEvents;               // Default VAD events
            std::optional<int> diarize;                 // Default diarization

            std::string toQueryString(const std::string &prefix = "/v1/listen") const
            {
                std::ostringstream oss;
                oss << prefix
                    << "?"
                    << "model=" << model
                    << "&language=" << language
                    << "&sample_rate=" << sampleRate
                    << "&encoding=" << encoding
                    << "&channels=" << channels;
                if (punctuate.has_value())
                {
                    oss << "&punctuate=" << (punctuate.value() ? "true" : "false");
                }
                if (interimResults.has_value())
                {
                    oss << "&interim_results=" << (interimResults.value() ? "true" : "false");
                }
                if (vadEvents.has_value())
                {
                    oss << "&vad_events=" << vadEvents.value();
                }
                if (diarize.has_value())
                {
                    oss << "&diarize=" << (diarize.value() ? "true" : "false");
                }
                if (multichannel.has_value())
                {
                    oss << "&multichannel=" << multichannel.value();
                }
                if (endpointing.has_value())
                {
                    oss << "&endpointing=" << endpointing.value();
                }
                if (utteranceEndMs.has_value())
                {
                    oss << "&utterance_end_ms=" << std::to_string(utteranceEndMs.value());
                }
                return oss.str();
            }
        };

        /**
         * Word
         * @note Represents a word in the transcription result.
         * This struct contains the word text, start and end timestamps, confidence score, and optional punctuated word and speaker information.
         * It provides a method to parse a word from a JSON object and print it in a human-readable format.
         */
        struct Word
        {
            std::string word;
            double start;
            double end;
            double confidence;
            std::optional<std::string> punctuated_word;
            std::optional<std::string> speaker;
            std::optional<std::string> speakerConfidence;

            static Word fromJson(const nlohmann::json &j)
            {
                Word w;
                w.word = j.value("word", "");
                w.start = j.value("start", 0.0);
                w.end = j.value("end", 0.0);
                w.confidence = j.value("confidence", 0.0);
                if (j.contains("punctuated_word"))
                {
                    w.punctuated_word = j["punctuated_word"];
                }
                if (j.contains("speaker"))
                {
                    w.speaker = j["speaker"];
                }
                if (j.contains("speaker_confidence"))
                {
                    w.speakerConfidence = j["speaker_confidence"];
                }
                return w;
            }

            void print() const
            {
                std::cout << "  Word: '" << word << "' [" << start << "s - " << end << "s] "
                          << "confidence: " << confidence;
                if (punctuated_word.has_value())
                {
                    std::cout << " punctuated: '" << punctuated_word.value() << "'";
                }
                if (speaker.has_value())
                {
                    std::cout << " speaker: " << speaker.value();
                }
                std::cout << std::endl;
            }
        };

        /**
         * Alternative
         * @note Represents an alternative transcription in the transcription result.
         * This struct contains the transcript text, confidence score, a list of words, and optional language information.
         * It provides a method to parse an alternative from a JSON object and print it in a human-readable format.
         */
        struct Alternative
        {
            std::string transcript;
            double confidence;
            std::vector<Word> words;
            std::optional<std::string> language;

            static Alternative fromJson(const nlohmann::json &j)
            {
                Alternative alt;
                alt.transcript = j.value("transcript", "");
                alt.confidence = j.value("confidence", 0.0);

                if (j.contains("words") && j["words"].is_array())
                {
                    for (const auto &wordJson : j["words"])
                    {
                        alt.words.push_back(Word::fromJson(wordJson));
                    }
                }

                if (j.contains("language"))
                {
                    alt.language = j["language"];
                }

                return alt;
            }

            void print() const
            {
                std::cout << "  Transcript: \"" << transcript << "\"" << std::endl;
                std::cout << "  Confidence: " << confidence << std::endl;
                if (language.has_value())
                {
                    std::cout << "  Language: " << language.value() << std::endl;
                }
                if (!words.empty())
                {
                    std::cout << "  Words (" << words.size() << "):" << std::endl;
                    for (const auto &word : words)
                    {
                        word.print();
                    }
                }
            }
        };

        /**
         * Channel
         * @note Represents a channel in the transcription result.
         * This struct contains a list of alternatives for the channel.
         * It provides a method to parse a channel from a JSON object and print it in a human-readable format.
         */
        struct Channel
        {
            std::vector<Alternative> alternatives;

            static Channel fromJson(const nlohmann::json &j)
            {
                Channel ch;
                if (j.contains("alternatives") && j["alternatives"].is_array())
                {
                    for (const auto &altJson : j["alternatives"])
                    {
                        ch.alternatives.push_back(Alternative::fromJson(altJson));
                    }
                }
                return ch;
            }

            void print() const
            {
                std::cout << "Channel with " << alternatives.size() << " alternatives:" << std::endl;
                for (size_t i = 0; i < alternatives.size(); ++i)
                {
                    std::cout << "Alternative " << i + 1 << ":" << std::endl;
                    alternatives[i].print();
                }
            }
        };

        /**
         * Metadata
         * @note Represents metadata about the connection to the Deepgram API.
         * This struct contains information such as transaction key, request ID, SHA256 hash, creation time, duration, and number of channels.
         * It provides a method to parse metadata from a JSON object and print it in a human-readable format.
         */
        struct Metadata
        {
            std::string transaction_key;
            std::string request_id;
            std::string sha256;
            std::string created;
            double duration;
            int channels;

            static Metadata fromJson(const nlohmann::json &j)
            {
                Metadata meta;
                meta.transaction_key = j.value("transaction_key", "");
                meta.request_id = j.value("request_id", "");
                meta.sha256 = j.value("sha256", "");
                meta.created = j.value("created", "");
                meta.duration = j.value("duration", 0.0);
                meta.channels = j.value("channels", 0);
                return meta;
            }

            void print() const
            {
                std::cout << "\n=== CONNECTION METADATA ===" << std::endl;
                std::cout << "Request ID: " << request_id << std::endl;
                std::cout << "Duration: " << duration << "s" << std::endl;
                std::cout << "Channels: " << channels << std::endl;
                std::cout << "Created: " << created << std::endl;
                std::cout << "==========================\n"
                          << std::endl;
            }
        };

        /**
         * Result types
         */
        namespace result
        {
            constexpr const char *RESULTS = "Results";
            constexpr const char *METADATA = "Metadata";
            constexpr const char *UTTERANCE_END = "UtteranceEnd";
            constexpr const char *SPEECH_STARTED = "SpeechStarted";
        };

        /**
         * TranscriptionResult
         * @note Represents a transcription result from the Deepgram API.
         * This struct contains the type of result, whether it is final, and the channel data.
         */
        struct TranscriptionResult
        {
            std::string type;
            Channel channel;
            bool isFinal = false; // Indicates if this result is final
            bool speech_final;
            bool isFromFinalize = false; // Indicates if this result is from a finalize event
            std::optional<double> duration;
            std::optional<double> start;

            static TranscriptionResult fromJson(const nlohmann::json &j)
            {
                TranscriptionResult result;
                result.type = j.value("type", "");
                result.isFinal = j.value("isFinal", false);
                result.speech_final = j.value("speech_final", false);
                result.isFromFinalize = j.value("is_from_finalize", false);

                if (j.contains("duration"))
                {
                    result.duration = j["duration"];
                }
                if (j.contains("start"))
                {
                    result.start = j["start"];
                }
                if (j.contains("channel"))
                {
                    result.channel = Channel::fromJson(j["channel"]);
                }

                return result;
            }

            void print() const
            {
                std::cout << "\n--- TRANSCRIPTION RESULT ---" << std::endl;
                std::cout << "Type: " << type << std::endl;
                std::cout << "Is Final: " << (isFinal ? "Yes" : "No") << std::endl;
                std::cout << "Speech Final: " << (speech_final ? "Yes" : "No") << std::endl;
                std::cout << "Is From Finalize: " << (isFromFinalize ? "Yes" : "No") << std::endl;
                if (duration.has_value())
                {
                    std::cout << "Duration: " << duration.value() << "s" << std::endl;
                }
                if (start.has_value())
                {
                    std::cout << "Start: " << start.value() << "s" << std::endl;
                }
                channel.print();
                std::cout << "----------------------------\n"
                          << std::endl;
            }

            // Helper methods for easy access
            std::string getBestTranscript() const
            {
                if (!channel.alternatives.empty())
                {
                    return channel.alternatives[0].transcript;
                }
                return "";
            }

            double getBestConfidence() const
            {
                if (!channel.alternatives.empty())
                {
                    return channel.alternatives[0].confidence;
                }
                return 0.0;
            }

            std::vector<Word> getWords() const
            {
                if (!channel.alternatives.empty())
                {
                    return channel.alternatives[0].words;
                }
                return {};
            }
        };

        /**
         * UtteranceEnd
         * @note Represents the end of an utterance in a transcription stream.
         * This struct contains the type of event and an optional timestamp indicating when the utterance ended.
         */
        struct UtteranceEnd
        {
            std::string type;
            std::optional<double> last_word_end;

            static UtteranceEnd fromJson(const nlohmann::json &j)
            {
                UtteranceEnd ue;
                ue.type = j.value("type", "");
                if (j.contains("last_word_end"))
                {
                    ue.last_word_end = j["last_word_end"];
                }
                return ue;
            }

            void print() const
            {
                std::cout << "🔚 UTTERANCE END";
                if (last_word_end.has_value())
                {
                    std::cout << " (last word at " << last_word_end.value() << "s)";
                }
                std::cout << std::endl;
            }
        };

        /**
         * SpeechStarted
         * @note Represents the start of speech detection in a transcription stream.
         * This struct contains the type of event and an optional timestamp indicating when the speech started.
         */
        struct SpeechStarted
        {
            std::string type;
            std::optional<double> timestamp;

            static SpeechStarted fromJson(const nlohmann::json &j)
            {
                SpeechStarted ss;
                ss.type = j.value("type", "");
                if (j.contains("timestamp"))
                {
                    ss.timestamp = j["timestamp"];
                }
                return ss;
            }

            void print() const
            {
                std::cout << "🎤 SPEECH STARTED";
                if (timestamp.has_value())
                {
                    std::cout << " (at " << timestamp.value() << "s)";
                }
                std::cout << std::endl;
            }
        };
    }
} // namespace deepgram
