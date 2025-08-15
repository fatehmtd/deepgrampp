#pragma once
#include <string>
#include <optional>
#include <sstream>
#include <nlohmann/json.hpp>

// Deepgram C++ SDK - Speak Module
// Provides access to Deepgram's text-to-speech capabilities
// Supports multiple languages and voices, including Aura 2 and Aura 1 models

namespace deepgram
{
    namespace speak
    {
        namespace models
        {
            /**
             * AURA 2
             * @note AURA 2 models are the latest generation of Deepgram's text-to-speech models.
             */
            namespace aura_2
            {
                /**
                 * English Voices - Supports American, British, Australian, Irish, and Filipino accents
                 * High-quality voices optimized for customer service, IVR, advertising, storytelling, and casual conversation
                 */
                namespace en
                {
                    // Filipino accent, Young Adult, Feminine - Engaging, Natural, Cheerful - Casual chat
                    constexpr const char *AMALTHEA = "aura-2-amalthea-en";

                    // American accent, Adult, Feminine - Casual, Expressive, Comfortable - Customer service, IVR
                    constexpr const char *ANDROMEDA = "aura-2-andromeda-en";

                    // American accent, Adult, Masculine - Confident, Comfortable, Casual - Casual chat
                    constexpr const char *APOLLO = "aura-2-apollo-en";

                    // American accent, Adult, Masculine - Natural, Smooth, Clear, Comfortable - Customer service, casual chat
                    constexpr const char *ARCAS = "aura-2-arcas-en";

                    // American accent, Adult, Masculine - Warm, Energetic, Caring - Casual chat
                    constexpr const char *ARIES = "aura-2-aries-en";

                    // American accent, Adult, Feminine - Clear, Confident, Knowledgeable, Energetic - Advertising
                    constexpr const char *ASTERIA = "aura-2-asteria-en";

                    // American accent, Mature, Feminine - Calm, Smooth, Professional - Storytelling
                    constexpr const char *ATHENA = "aura-2-athena-en";

                    // American accent, Mature, Masculine - Enthusiastic, Confident, Approachable, Friendly - Advertising
                    constexpr const char *ATLAS = "aura-2-atlas-en";

                    // American accent, Adult, Feminine - Cheerful, Expressive, Energetic - Interview
                    constexpr const char *AURORA = "aura-2-aurora-en";

                    // American accent, Adult, Feminine - Clear, Energetic, Professional, Smooth - IVR
                    constexpr const char *CALLISTA = "aura-2-callista-en";

                    // American accent, Adult, Feminine - Smooth, Melodic, Caring - Storytelling
                    constexpr const char *CORA = "aura-2-cora-en";

                    // American accent, Young Adult, Feminine - Approachable, Warm, Polite - Storytelling
                    constexpr const char *CORDELIA = "aura-2-cordelia-en";

                    // American accent, Young Adult, Feminine - Casual, Friendly, Cheerful, Breathy - Interview
                    constexpr const char *DELIA = "aura-2-delia-en";

                    // British accent, Adult, Masculine - Warm, Approachable, Trustworthy, Baritone - Storytelling
                    constexpr const char *DRACO = "aura-2-draco-en";

                    // American accent, Adult, Feminine - Professional, Engaging, Knowledgeable - IVR, advertising, customer service
                    constexpr const char *ELECTRA = "aura-2-electra-en";

                    // American accent, Adult, Feminine - Empathetic, Clear, Calm, Confident - Customer service
                    constexpr const char *HARMONIA = "aura-2-harmonia-en";

                    // American accent, Adult, Feminine - Caring, Natural, Positive, Friendly, Raspy - IVR, casual chat
                    constexpr const char *HELENA = "aura-2-helena-en";

                    // American accent, Adult, Feminine - Smooth, Warm, Professional - Informative
                    constexpr const char *HERA = "aura-2-hera-en";

                    // American accent, Adult, Masculine - Expressive, Engaging, Professional - Informative
                    constexpr const char *HERMES = "aura-2-hermes-en";

                    // Australian accent, Adult, Masculine - Caring, Warm, Empathetic - Interview
                    constexpr const char *HYPERION = "aura-2-hyperion-en";

                    // American accent, Young Adult, Feminine - Cheerful, Positive, Approachable - IVR, advertising, customer service
                    constexpr const char *IRIS = "aura-2-iris-en";

                    // American accent, Adult, Feminine - Southern, Smooth, Trustworthy - Storytelling
                    constexpr const char *JANUS = "aura-2-janus-en";

                    // American accent, Adult, Feminine - Natural, Engaging, Melodic, Breathy - Interview
                    constexpr const char *JUNO = "aura-2-juno-en";

                    // American accent, Adult, Masculine - Expressive, Knowledgeable, Baritone - Informative
                    constexpr const char *JUPITER = "aura-2-jupiter-en";

                    // American accent, Young Adult, Feminine - Friendly, Natural, Engaging - IVR
                    constexpr const char *LUNA = "aura-2-luna-en";

                    // American accent, Adult, Masculine - Smooth, Patient, Trustworthy, Baritone - Customer service
                    constexpr const char *MARS = "aura-2-mars-en";

                    // American accent, Adult, Feminine - Positive, Friendly, Natural - Storytelling
                    constexpr const char *MINERVA = "aura-2-minerva-en";

                    // American accent, Adult, Masculine - Professional, Patient, Polite - Customer service
                    constexpr const char *NEPTUNE = "aura-2-neptune-en";

                    // American accent, Adult, Masculine - Calm, Smooth, Comfortable, Professional - Advertising
                    constexpr const char *ODYSSEUS = "aura-2-odysseus-en";

                    // American accent, Adult, Feminine - Expressive, Enthusiastic, Cheerful - Interview
                    constexpr const char *OPHELIA = "aura-2-ophelia-en";

                    // American accent, Adult, Masculine - Approachable, Comfortable, Calm, Polite - Informative
                    constexpr const char *ORION = "aura-2-orion-en";

                    // American accent, Adult, Masculine - Professional, Clear, Confident, Trustworthy - Customer service, storytelling
                    constexpr const char *ORPHEUS = "aura-2-orpheus-en";

                    // British accent, Adult, Feminine - Smooth, Calm, Melodic, Breathy - IVR, informative
                    constexpr const char *PANDORA = "aura-2-pandora-en";

                    // American accent, Adult, Feminine - Energetic, Warm, Casual - Customer service
                    constexpr const char *PHOEBE = "aura-2-phoebe-en";

                    // American accent, Adult, Masculine - Smooth, Calm, Empathetic, Baritone - Interview, storytelling
                    constexpr const char *PLUTO = "aura-2-pluto-en";

                    // American accent, Adult, Masculine - Knowledgeable, Confident, Baritone - Customer service
                    constexpr const char *SATURN = "aura-2-saturn-en";

                    // American accent, Adult, Feminine - Expressive, Engaging, Energetic - Informative
                    constexpr const char *SELENE = "aura-2-selene-en";

                    // American accent, Adult, Feminine - Clear, Confident, Energetic, Enthusiastic - Casual chat, customer service, IVR
                    constexpr const char *THALIA = "aura-2-thalia-en";

                    // Australian accent, Adult, Feminine - Expressive, Polite, Sincere - Informative
                    constexpr const char *THEIA = "aura-2-theia-en";

                    // American accent, Adult, Feminine - Natural, Expressive, Patient, Empathetic - Customer service, interview, storytelling
                    constexpr const char *VESTA = "aura-2-vesta-en";

                    // American accent, Adult, Masculine - Deep, Trustworthy, Smooth - IVR
                    constexpr const char *ZEUS = "aura-2-zeus-en";
                }

                /**
                 * Spanish Voices - Supports Mexican, Peninsular, Colombian, and Latin American accents
                 * Optimized for customer service, IVR, casual conversation, interviews, and storytelling
                 * Voices marked with (Codeswitching) can seamlessly switch between English and Spanish
                 */
                namespace es
                {
                    // Peninsular accent, Adult, Masculine - Calm, Professional, Clear, Knowledgeable, Approachable - Interview, Customer Service
                    constexpr const char *ALVARO = "aura-2-alvaro-es";

                    // Latin American accent, Adult, Masculine (Codeswitching) - Expressive, Enthusiastic, Confident, Casual, Comfortable - Casual Chat, Informative
                    constexpr const char *AQUILA = "aura-2-aquila-es";

                    // Peninsular accent, Adult, Feminine (Codeswitching) - Professional, Raspy, Energetic, Breathy, Confident - Interview, Customer Service, IVR
                    constexpr const char *CARINA = "aura-2-carina-es";

                    // Colombian accent, Young Adult, Feminine - Clear, Energetic, Positive, Friendly, Enthusiastic - Casual Chat, Advertising, IVR
                    constexpr const char *CELESTE = "aura-2-celeste-es";

                    // Peninsular accent, Adult, Feminine (Codeswitching) - Professional, Confident, Expressive, Polite, Knowledgeable - Storytelling, Advertising
                    constexpr const char *DIANA = "aura-2-diana-es";

                    // Mexican accent, Mature, Feminine - Approachable, Natural, Calm, Comfortable, Expressive - Casual Chat, Interview
                    constexpr const char *ESTRELLA = "aura-2-estrella-es";

                    // Latin American accent, Adult, Masculine (Codeswitching) - Approachable, Professional, Friendly, Comfortable, Calm - Casual Chat, IVR, Storytelling
                    constexpr const char *JAVIER = "aura-2-javier-es";

                    // Peninsular accent, Adult, Masculine - Calm, Professional, Approachable, Clear, Confident - Casual Chat, Customer Service
                    constexpr const char *NESTOR = "aura-2-nestor-es";

                    // Latin American accent, Young Adult, Feminine (Codeswitching) - Approachable, Casual, Friendly, Calm, Positive - Customer Service, Informative
                    constexpr const char *SELENA = "aura-2-selena-es";

                    // Mexican accent, Adult, Masculine - Calm, Professional, Comfortable, Empathetic, Baritone - Casual Chat, Interview
                    constexpr const char *SIRIO = "aura-2-sirio-es";
                }
            }

            namespace aura
            {
                /**
                 * Aura 1 English Voices (Legacy) - First generation voices
                 * Good quality but superseded by Aura 2 for most use cases
                 */
                namespace en
                {
                    // Irish accent, Adult, Masculine - Warm, Friendly, Natural - Storytelling
                    constexpr const char *ANGUS = "aura-angus-en";

                    // American accent, Adult, Masculine - Natural, Smooth, Clear, Comfortable - Customer service, casual chat
                    constexpr const char *ARCAS = "aura-arcas-en";

                    // American accent, Adult, Feminine - Clear, Confident, Knowledgeable, Energetic - Advertising (Default voice)
                    constexpr const char *ASTERIA = "aura-asteria-en";

                    // British accent, Mature, Feminine - Calm, Smooth, Professional - Storytelling
                    constexpr const char *ATHENA = "aura-athena-en";

                    // British accent, Adult, Masculine - Professional, Clear, Confident - Customer service
                    constexpr const char *HELIOS = "aura-helios-en";

                    // American accent, Adult, Feminine - Smooth, Warm, Professional - Informative
                    constexpr const char *HERA = "aura-hera-en";

                    // American accent, Young Adult, Feminine - Friendly, Natural, Engaging - IVR
                    constexpr const char *LUNA = "aura-luna-en";

                    // American accent, Adult, Masculine - Approachable, Comfortable, Calm, Polite - Informative
                    constexpr const char *ORION = "aura-orion-en";

                    // American accent, Adult, Masculine - Professional, Clear, Confident, Trustworthy - Customer service, storytelling
                    constexpr const char *ORPHEUS = "aura-orpheus-en";

                    // American accent, Adult, Masculine - Confident, Professional, Clear - Customer service
                    constexpr const char *PERSEUS = "aura-perseus-en";

                    // American accent, Adult, Feminine - Clear, Professional, Engaging - Customer service
                    constexpr const char *STELLA = "aura-stella-en";

                    // American accent, Adult, Masculine - Deep, Trustworthy, Smooth - IVR
                    constexpr const char *ZEUS = "aura-zeus-en";
                }
            }

            // Featured voices for quick reference
            namespace featured
            {
                namespace en
                {
                    // Top English voices recommended by Deepgram for their versatility and quality
                    constexpr const char *THALIA = aura_2::en::THALIA;       // Clear, Confident, Energetic
                    constexpr const char *ANDROMEDA = aura_2::en::ANDROMEDA; // Casual, Expressive
                    constexpr const char *HELENA = aura_2::en::HELENA;       // Caring, Natural, Positive
                    constexpr const char *APOLLO = aura_2::en::APOLLO;       // Confident, Comfortable
                    constexpr const char *ARCAS = aura_2::en::ARCAS;         // Natural, Smooth, Clear
                    constexpr const char *ARIES = aura_2::en::ARIES;         // Warm, Energetic
                }

                namespace es
                {
                    // Top Spanish voices recommended by Deepgram
                    constexpr const char *CELESTE = aura_2::es::CELESTE;   // Clear, Energetic, Positive
                    constexpr const char *ESTRELLA = aura_2::es::ESTRELLA; // Approachable, Natural, Calm
                    constexpr const char *NESTOR = aura_2::es::NESTOR;     // Calm, Professional
                }
            }
        }

        /**
         * Audio Encoding Types
         * @note Supported audio encoding types for text-to-speech.
         *       See https://developers.deepgram.com/docs/sample-rate for more details.
         */
        namespace encoding
        {
            // 16-bit, little-endian, signed PCM WAV data.	REST ,Streaming
            constexpr const char *LINEAR_16 = "linear16";
            // Mu-law encoded WAV data.	REST ,Streaming
            constexpr const char *MULAW = "mulaw";
            // A-law encoded WAV data.	REST ,Streaming
            constexpr const char *ALAW = "alaw";
            // 32-bit, little-endian, floating-point PCM WAV data.	REST ,Streaming
            constexpr const char *LINEAR_32 = "linear32";
            // Free Lossless Audio Codec (FLAC) encoded data.	REST ,Streaming
            constexpr const char *FLAC = "flac";
            // Ogg Opus codec.	REST ,Streaming
            constexpr const char *OPUS = "opus";
            // Advanced Audio Coding format
            constexpr const char *AAC = "aac";
            // MP3 audio compression format.	REST
            constexpr const char *MP3 = "mp3";
        }

        /**
         * LiveSpeakConfig
         * @note Configuration options for the LiveSpeak service.
         *       Allows customization of model, encoding, sample rate, bitrate, and container format.
         *       These options can be used to tailor the text-to-speech output to specific requirements.
         */
        struct LiveSpeakConfig
        {
            std::string model = models::aura_2::en::THALIA; // Default model
            std::string encoding = encoding::LINEAR_16;      // Encoding type, e.g., "linear16", "opus"
            std::optional<int> sampleRate = 16000;         // Sample rate in Hz, default is 16000
            std::optional<int> bitrate;                     // Bitrate in bits per second
            std::optional<std::string> container;           // Container format, e.g., "wav", "ogg"
            std::optional<bool> mip_opt_out;

            std::string toQueryString(const std::string& prefix = "/v1/speak") const {
                std::ostringstream oss;
                oss << prefix
                << "?"
                << "model=" << model
                << "&encoding=" << encoding;
                if(sampleRate.has_value()) {
                    oss << "&sample_rate=" << sampleRate.value();
                }
                if(mip_opt_out.has_value()) {
                    oss << "&mip_opt_out=" << (mip_opt_out.value() ? "true" : "false");
                }
                return oss.str();
            }
        };

        /**
         * SpeakControl
         * Represents the control information sent to the server.
         */
        namespace control
        {
            // Flush control message
            constexpr const char *FLUSH = R"({"type" : "Flush"})";
            // Clear control message
            constexpr const char *CLEAR = R"({"type" : "Clear"})";
            // Close control message
            constexpr const char *CLOSE = R"({"type" : "Close"})";
        };

        /**
         * SpeakControlResponse
         * Represents the control response information returned by the server.
         */
        struct SpeakControlResponse
        {
            std::string type;
            std::optional<int> sequenceId;

            static SpeakControlResponse fromJson(const nlohmann::json &j)
            {
                SpeakControlResponse response;
                j.at("type").get_to(response.type);
                if (j.contains("sequenceId"))
                {
                    response.sequenceId = j.at("sequenceId").get<int>();
                }
                return response;
            }

            std::string toString() const {
                std::ostringstream oss;
                oss << "SpeakControlResponse {"
                    << " type: " << type;
                if (sequenceId) {
                    oss << ", sequenceId: " << *sequenceId;
                }
                oss << " }";
                return oss.str();
            }
        };

        /**
         * SpeakCloseFrame
         * Represents the close frame information returned by the server.
         */
        struct SpeakCloseFrame
        {
            std::string code;
            std::string payload;

            static SpeakCloseFrame fromJson(const nlohmann::json &j)
            {
                SpeakCloseFrame frame;
                j.at("code").get_to(frame.code);
                j.at("payload").get_to(frame.payload);
                return frame;
            }

            std::string toString() const {
                std::ostringstream oss;
                oss << "SpeakCloseFrame {"
                    << " code: " << code
                    << ", payload: " << payload
                    << " }";
                return oss.str();
            }
        };

        /**
         * MetadataResponse
         * Represents the metadata information returned by the server.
         */
        struct MetadataResponse 
        {
            std::string type;
            std::optional<std::string> request_id;
            std::optional<std::string> model_name;
            std::optional<std::string> model_version;
            std::optional<std::string> model_uuid;

            static MetadataResponse fromJson(const nlohmann::json &j) {
                MetadataResponse response;
                j.at("type").get_to(response.type);
                if (j.contains("request_id")) {
                    response.request_id = j.at("request_id").get<std::string>();
                }
                if (j.contains("model_name")) {
                    response.model_name = j.at("model_name").get<std::string>();
                }
                if (j.contains("model_version")) {
                    response.model_version = j.at("model_version").get<std::string>();
                }
                if (j.contains("model_uuid")) {
                    response.model_uuid = j.at("model_uuid").get<std::string>();
                }
                return response;
            }

            std::string toString() const {
                std::ostringstream oss;
                oss << "MetadataResponse {"
                    << " type: " << type;
                if (request_id) {
                    oss << ", request_id: " << *request_id;
                }
                if (model_name) {
                    oss << ", model_name: " << *model_name;
                }
                if (model_version) {
                    oss << ", model_version: " << *model_version;
                }
                if (model_uuid) {
                    oss << ", model_uuid: " << *model_uuid;
                }
                oss << " }";
                return oss.str();
            }
        };
    }
} // namespace deepgram