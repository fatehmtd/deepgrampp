#pragma once

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

namespace deepgram
{
    namespace control
    {
        /**
         * Control messages for the Deepgram WebSocket API.
         *
         * @note These messages are used to control the WebSocket connection and stream.
         */
        const std::string CLOSE_MESSAGE(R"({"type": "CloseStream"})");
        const std::string KEEPALIVE_MESSAGE(R"({"type": "KeepAlive"})");
        const std::string FINALIZE_MESSAGE(R"({"type": "Finalize"})");
    }

    namespace models
    {
        /**
         * NOVA3
         *
         * @note NOVA3 models are the latest generation of Deepgram's speech-to-text models.
         *       They are designed to provide high accuracy and performance for a wide range of applications.
         *       See https://developers.deepgram.com/docs/nova3 for more details.
         */
        const std::string NOVA3_GENERAL = "nova3-general";
        const std::string NOVA3_MEDICAL = "nova3-medical";

        /**
         * NOVA2
         *
         * @note NOVA2 models are deprecated and will be removed in a future release.
         *       Use NOVA3 models instead.
         *       See https://developers.deepgram.com/docs/nova2-deprecation for more details.
         *       @deprecated NOVA2 models are deprecated and will be removed in a future release.
         *       Use NOVA3 models instead.
         */
        // general: (Default) Optimized for everyday audio processing.
        const std::string NOVA2_GENERAL = "nova2-general";
        const std::string NOVA2_MEETING = "nova2-meeting";
        // Optimized for low-bandwidth audio phone calls.
        const std::string NOVA2_PHONECALL = "nova2-phonecall";
        const std::string NOVA2_VOICEMAIL = "nova2-voicemail";
        const std::string NOVA2_FINANCE = "nova2-finance";
        const std::string NOVA2_CONVERSATIONALAI = "nova2-conversationalai";
        const std::string NOVA2_VIDEO = "nova2-video";
        const std::string NOVA2_DRIVETHRU = "nova2-drivethru";
        const std::string NOVA2_MEDICAL = "nova2-medical";
        const std::string NOVA2_AUTOMOTIVE = "nova2-automotive";
        const std::string NOVA2_ATC = "nova2-atc";

        /**
         * NOVA
         *
         * @note NOVA models are the original Deepgram speech-to-text models.
         *       They are still available for use, but are not recommended for new applications.
         *       See https://developers.deepgram.com/docs/nova-models for more details.
         *       @deprecated NOVA models are deprecated and will be removed in a future release.
         *       Use NOVA3 models instead.
         */
        // general: (Default) Optimized for everyday audio processing.
        const std::string NOVA_GENERAL = "nova-general";
        // Optimized for low-bandwidth audio phone calls.
        const std::string NOVA_PHONECALL = "nova-phonecall";

        /**
         * BASE
         * @note BASE models are the original Deepgram speech-to-text models.
         *       They are still available for use, but are not recommended for new applications.
         *       See https://developers.deepgram.com/docs/base-models for more details.
         */
        // general: (Default) Optimized for everyday audio processing.
        const std::string BASE_GENERAL = "base-general";
        // Optimized for conference room settings, which include multiple speakers with a single microphone.
        const std::string BASE_MEETING = "base-meeting";
        // Optimized for low-bandwidth audio phone calls.
        const std::string BASE_PHONECALL = "base-phonecall";
        // voicemail: Optimized for low-bandwidth audio clips with a single speaker. Derived from the phonecall model.
        const std::string BASE_VOICEMAIL = "base-voicemail";
        // Optimized for multiple speakers with varying audio quality, such as might be found on a typical earnings call. Vocabulary is heavily finance oriented.
        const std::string BASE_FINANCE = "base-finance";
        // Optimized for use cases in which a human is talking to an automated bot, such as IVR, a voice assistant, or an automated kiosk.
        const std::string BASE_CONVERSATIONALAI = "base-conversationalai";
        // Optimized for audio sourced from videos.
        const std::string BASE_VIDEO = "base-video";
    }

    struct Word
    {
        std::string word;
        double start;
        double end;
        double confidence;
        std::optional<std::string> punctuated_word;
        std::optional<std::string> speaker;

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

    struct TranscriptionResult
    {
        std::string type;
        Channel channel;
        bool is_final;
        bool speech_final;
        std::optional<double> duration;
        std::optional<double> start;

        static TranscriptionResult fromJson(const nlohmann::json &j)
        {
            TranscriptionResult result;
            result.type = j.value("type", "");
            result.is_final = j.value("is_final", false);
            result.speech_final = j.value("speech_final", false);

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
            std::cout << "Is Final: " << (is_final ? "Yes" : "No") << std::endl;
            std::cout << "Speech Final: " << (speech_final ? "Yes" : "No") << std::endl;
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
} // namespace deepgram