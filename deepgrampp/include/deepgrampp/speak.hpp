#pragma once
#include <string>
#include <optional>

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
                    constexpr char *AMALTHEA = "aura-2-amalthea-en";

                    // American accent, Adult, Feminine - Casual, Expressive, Comfortable - Customer service, IVR
                    constexpr char *ANDROMEDA = "aura-2-andromeda-en";

                    // American accent, Adult, Masculine - Confident, Comfortable, Casual - Casual chat
                    constexpr char *APOLLO = "aura-2-apollo-en";

                    // American accent, Adult, Masculine - Natural, Smooth, Clear, Comfortable - Customer service, casual chat
                    constexpr char *ARCAS = "aura-2-arcas-en";

                    // American accent, Adult, Masculine - Warm, Energetic, Caring - Casual chat
                    constexpr char *ARIES = "aura-2-aries-en";

                    // American accent, Adult, Feminine - Clear, Confident, Knowledgeable, Energetic - Advertising
                    constexpr char *ASTERIA = "aura-2-asteria-en";

                    // American accent, Mature, Feminine - Calm, Smooth, Professional - Storytelling
                    constexpr char *ATHENA = "aura-2-athena-en";

                    // American accent, Mature, Masculine - Enthusiastic, Confident, Approachable, Friendly - Advertising
                    constexpr char *ATLAS = "aura-2-atlas-en";

                    // American accent, Adult, Feminine - Cheerful, Expressive, Energetic - Interview
                    constexpr char *AURORA = "aura-2-aurora-en";

                    // American accent, Adult, Feminine - Clear, Energetic, Professional, Smooth - IVR
                    constexpr char *CALLISTA = "aura-2-callista-en";

                    // American accent, Adult, Feminine - Smooth, Melodic, Caring - Storytelling
                    constexpr char *CORA = "aura-2-cora-en";

                    // American accent, Young Adult, Feminine - Approachable, Warm, Polite - Storytelling
                    constexpr char *CORDELIA = "aura-2-cordelia-en";

                    // American accent, Young Adult, Feminine - Casual, Friendly, Cheerful, Breathy - Interview
                    constexpr char *DELIA = "aura-2-delia-en";

                    // British accent, Adult, Masculine - Warm, Approachable, Trustworthy, Baritone - Storytelling
                    constexpr char *DRACO = "aura-2-draco-en";

                    // American accent, Adult, Feminine - Professional, Engaging, Knowledgeable - IVR, advertising, customer service
                    constexpr char *ELECTRA = "aura-2-electra-en";

                    // American accent, Adult, Feminine - Empathetic, Clear, Calm, Confident - Customer service
                    constexpr char *HARMONIA = "aura-2-harmonia-en";

                    // American accent, Adult, Feminine - Caring, Natural, Positive, Friendly, Raspy - IVR, casual chat
                    constexpr char *HELENA = "aura-2-helena-en";

                    // American accent, Adult, Feminine - Smooth, Warm, Professional - Informative
                    constexpr char *HERA = "aura-2-hera-en";

                    // American accent, Adult, Masculine - Expressive, Engaging, Professional - Informative
                    constexpr char *HERMES = "aura-2-hermes-en";

                    // Australian accent, Adult, Masculine - Caring, Warm, Empathetic - Interview
                    constexpr char *HYPERION = "aura-2-hyperion-en";

                    // American accent, Young Adult, Feminine - Cheerful, Positive, Approachable - IVR, advertising, customer service
                    constexpr char *IRIS = "aura-2-iris-en";

                    // American accent, Adult, Feminine - Southern, Smooth, Trustworthy - Storytelling
                    constexpr char *JANUS = "aura-2-janus-en";

                    // American accent, Adult, Feminine - Natural, Engaging, Melodic, Breathy - Interview
                    constexpr char *JUNO = "aura-2-juno-en";

                    // American accent, Adult, Masculine - Expressive, Knowledgeable, Baritone - Informative
                    constexpr char *JUPITER = "aura-2-jupiter-en";

                    // American accent, Young Adult, Feminine - Friendly, Natural, Engaging - IVR
                    constexpr char *LUNA = "aura-2-luna-en";

                    // American accent, Adult, Masculine - Smooth, Patient, Trustworthy, Baritone - Customer service
                    constexpr char *MARS = "aura-2-mars-en";

                    // American accent, Adult, Feminine - Positive, Friendly, Natural - Storytelling
                    constexpr char *MINERVA = "aura-2-minerva-en";

                    // American accent, Adult, Masculine - Professional, Patient, Polite - Customer service
                    constexpr char *NEPTUNE = "aura-2-neptune-en";

                    // American accent, Adult, Masculine - Calm, Smooth, Comfortable, Professional - Advertising
                    constexpr char *ODYSSEUS = "aura-2-odysseus-en";

                    // American accent, Adult, Feminine - Expressive, Enthusiastic, Cheerful - Interview
                    constexpr char *OPHELIA = "aura-2-ophelia-en";

                    // American accent, Adult, Masculine - Approachable, Comfortable, Calm, Polite - Informative
                    constexpr char *ORION = "aura-2-orion-en";

                    // American accent, Adult, Masculine - Professional, Clear, Confident, Trustworthy - Customer service, storytelling
                    constexpr char *ORPHEUS = "aura-2-orpheus-en";

                    // British accent, Adult, Feminine - Smooth, Calm, Melodic, Breathy - IVR, informative
                    constexpr char *PANDORA = "aura-2-pandora-en";

                    // American accent, Adult, Feminine - Energetic, Warm, Casual - Customer service
                    constexpr char *PHOEBE = "aura-2-phoebe-en";

                    // American accent, Adult, Masculine - Smooth, Calm, Empathetic, Baritone - Interview, storytelling
                    constexpr char *PLUTO = "aura-2-pluto-en";

                    // American accent, Adult, Masculine - Knowledgeable, Confident, Baritone - Customer service
                    constexpr char *SATURN = "aura-2-saturn-en";

                    // American accent, Adult, Feminine - Expressive, Engaging, Energetic - Informative
                    constexpr char *SELENE = "aura-2-selene-en";

                    // American accent, Adult, Feminine - Clear, Confident, Energetic, Enthusiastic - Casual chat, customer service, IVR
                    constexpr char *THALIA = "aura-2-thalia-en";

                    // Australian accent, Adult, Feminine - Expressive, Polite, Sincere - Informative
                    constexpr char *THEIA = "aura-2-theia-en";

                    // American accent, Adult, Feminine - Natural, Expressive, Patient, Empathetic - Customer service, interview, storytelling
                    constexpr char *VESTA = "aura-2-vesta-en";

                    // American accent, Adult, Masculine - Deep, Trustworthy, Smooth - IVR
                    constexpr char *ZEUS = "aura-2-zeus-en";
                }

                /**
                 * Spanish Voices - Supports Mexican, Peninsular, Colombian, and Latin American accents
                 * Optimized for customer service, IVR, casual conversation, interviews, and storytelling
                 * Voices marked with (Codeswitching) can seamlessly switch between English and Spanish
                 */
                namespace es
                {
                    // Peninsular accent, Adult, Masculine - Calm, Professional, Clear, Knowledgeable, Approachable - Interview, Customer Service
                    constexpr char *ALVARO = "aura-2-alvaro-es";

                    // Latin American accent, Adult, Masculine (Codeswitching) - Expressive, Enthusiastic, Confident, Casual, Comfortable - Casual Chat, Informative
                    constexpr char *AQUILA = "aura-2-aquila-es";

                    // Peninsular accent, Adult, Feminine (Codeswitching) - Professional, Raspy, Energetic, Breathy, Confident - Interview, Customer Service, IVR
                    constexpr char *CARINA = "aura-2-carina-es";

                    // Colombian accent, Young Adult, Feminine - Clear, Energetic, Positive, Friendly, Enthusiastic - Casual Chat, Advertising, IVR
                    constexpr char *CELESTE = "aura-2-celeste-es";

                    // Peninsular accent, Adult, Feminine (Codeswitching) - Professional, Confident, Expressive, Polite, Knowledgeable - Storytelling, Advertising
                    constexpr char *DIANA = "aura-2-diana-es";

                    // Mexican accent, Mature, Feminine - Approachable, Natural, Calm, Comfortable, Expressive - Casual Chat, Interview
                    constexpr char *ESTRELLA = "aura-2-estrella-es";

                    // Latin American accent, Adult, Masculine (Codeswitching) - Approachable, Professional, Friendly, Comfortable, Calm - Casual Chat, IVR, Storytelling
                    constexpr char *JAVIER = "aura-2-javier-es";

                    // Peninsular accent, Adult, Masculine - Calm, Professional, Approachable, Clear, Confident - Casual Chat, Customer Service
                    constexpr char *NESTOR = "aura-2-nestor-es";

                    // Latin American accent, Young Adult, Feminine (Codeswitching) - Approachable, Casual, Friendly, Calm, Positive - Customer Service, Informative
                    constexpr char *SELENA = "aura-2-selena-es";

                    // Mexican accent, Adult, Masculine - Calm, Professional, Comfortable, Empathetic, Baritone - Casual Chat, Interview
                    constexpr char *SIRIO = "aura-2-sirio-es";
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
                    constexpr char *ANGUS = "aura-angus-en";

                    // American accent, Adult, Masculine - Natural, Smooth, Clear, Comfortable - Customer service, casual chat
                    constexpr char *ARCAS = "aura-arcas-en";

                    // American accent, Adult, Feminine - Clear, Confident, Knowledgeable, Energetic - Advertising (Default voice)
                    constexpr char *ASTERIA = "aura-asteria-en";

                    // British accent, Mature, Feminine - Calm, Smooth, Professional - Storytelling
                    constexpr char *ATHENA = "aura-athena-en";

                    // British accent, Adult, Masculine - Professional, Clear, Confident - Customer service
                    constexpr char *HELIOS = "aura-helios-en";

                    // American accent, Adult, Feminine - Smooth, Warm, Professional - Informative
                    constexpr char *HERA = "aura-hera-en";

                    // American accent, Young Adult, Feminine - Friendly, Natural, Engaging - IVR
                    constexpr char *LUNA = "aura-luna-en";

                    // American accent, Adult, Masculine - Approachable, Comfortable, Calm, Polite - Informative
                    constexpr char *ORION = "aura-orion-en";

                    // American accent, Adult, Masculine - Professional, Clear, Confident, Trustworthy - Customer service, storytelling
                    constexpr char *ORPHEUS = "aura-orpheus-en";

                    // American accent, Adult, Masculine - Confident, Professional, Clear - Customer service
                    constexpr char *PERSEUS = "aura-perseus-en";

                    // American accent, Adult, Feminine - Clear, Professional, Engaging - Customer service
                    constexpr char *STELLA = "aura-stella-en";

                    // American accent, Adult, Masculine - Deep, Trustworthy, Smooth - IVR
                    constexpr char *ZEUS = "aura-zeus-en";
                }
            }

            // Featured voices for quick reference
            namespace featured
            {
                namespace en
                {
                    // Top English voices recommended by Deepgram for their versatility and quality
                    constexpr char *THALIA = aura_2::en::THALIA;       // Clear, Confident, Energetic
                    constexpr char *ANDROMEDA = aura_2::en::ANDROMEDA; // Casual, Expressive
                    constexpr char *HELENA = aura_2::en::HELENA;       // Caring, Natural, Positive
                    constexpr char *APOLLO = aura_2::en::APOLLO;       // Confident, Comfortable
                    constexpr char *ARCAS = aura_2::en::ARCAS;         // Natural, Smooth, Clear
                    constexpr char *ARIES = aura_2::en::ARIES;         // Warm, Energetic
                }

                namespace es
                {
                    // Top Spanish voices recommended by Deepgram
                    constexpr char *CELESTE = aura_2::es::CELESTE;   // Clear, Energetic, Positive
                    constexpr char *ESTRELLA = aura_2::es::ESTRELLA; // Approachable, Natural, Calm
                    constexpr char *NESTOR = aura_2::es::NESTOR;     // Calm, Professional
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
            constexpr char *LINEAR16 = "linear16";
            // Mu-law encoded WAV data.	REST ,Streaming
            constexpr char *MULAW = "mulaw";
            // A-law encoded WAV data.	REST ,Streaming
            constexpr char *ALAW = "alaw";
            // 32-bit, little-endian, floating-point PCM WAV data.	REST ,Streaming
            constexpr char *LINEAR32 = "linear32";
            // Free Lossless Audio Codec (FLAC) encoded data.	REST ,Streaming
            constexpr char *FLAC = "flac";
            // Ogg Opus codec.	REST ,Streaming
            constexpr char *OPUS = "opus";
            // Advanced Audio Coding format
            constexpr char *AAC = "aac";
            // MP3 audio compression format.	REST
            constexpr char *MP3 = "mp3";
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
            std::string encoding = encoding::LINEAR16;      // Encoding type, e.g., "linear16", "opus"
            std::optional<int> sample_rate = 16000;         // Sample rate in Hz, default is 16000
            std::optional<int> bitrate;                     // Bitrate in bits per second
            std::optional<std::string> container;           // Container format, e.g., "wav", "ogg"
        };
    }
} // namespace deepgram