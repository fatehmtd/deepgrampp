#ifndef DEEPGRAMPP_LISTEN_FLUX_HPP
#define DEEPGRAMPP_LISTEN_FLUX_HPP

#include <deepgrampp_lib_export.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <sstream>

/**
 * @brief Deepgram Listen Flux API client.
 * This client provides functionality to connect to Deepgram's Listen Flux API, see https://developers.deepgram.com/reference/speech-to-text/listen-flux
 */
namespace deepgram {
    namespace listen {
        namespace flux {

            namespace model {
                // @note Update this as new models are released
                constexpr const char* FLUX_GENERAL_EN = "flux-general-en";
            }

            namespace encoding {
                // @note Update this as new encodings are supported
                constexpr const char* LINEAR16 = "linear16";
            }

            /**
             * @brief FluxQueryParams parameters for the Deepgram Listen Flux API.
             */
            struct FluxQueryParams {
                std::string model = model::FLUX_GENERAL_EN;
                std::string encoding = encoding::LINEAR16;
                int sample_rate;
                std::optional<double> eager_eot_threshold;
                std::optional<double> eot_threshold;
                std::optional<int> eot_timeout_ms;
                std::vector<std::string> keyterm;
                std::optional<bool> mip_opt_out;
                std::optional<std::string> tag;

                std::string toQueryString() const {
                    std::ostringstream query;
                    query << "model=" << model
                        << "&encoding=" << encoding
                        << "&sample_rate=" << sample_rate;

                    if (eager_eot_threshold.has_value()) {
                        query << "&eager_eot_threshold=" << eager_eot_threshold.value();
                    }
                    if (eot_threshold.has_value()) {
                        query << "&eot_threshold=" << eot_threshold.value();
                    }
                    if (eot_timeout_ms.has_value()) {
                        query << "&eot_timeout_ms=" << eot_timeout_ms.value();
                    }
                    for (const auto& kt : keyterm) {
                        query << "&keyterm=" << kt;
                    }
                    if (mip_opt_out.has_value()) {
                        query << "&mip_opt_out=" << (mip_opt_out.value() ? std::string("true") : std::string("false"));
                    }
                    if (tag.has_value()) {
                        query << "&tag=" << tag.value();
                    }

                    return query.str();
                }
            };

            namespace event {
                /**
                 * @brief Event types for the Deepgram Listen Flux API.
                 */
                namespace type {
                    constexpr const char* CONNECTED = "Connected";
                    constexpr const char* CLOSE_STREAM = "CloseStream";
                    constexpr const char* TURN_INFO = "TurnInfo";
                    constexpr const char* FATAL_ERROR = "Error";
                };

                /**
                 * @brief Represents a Connected event from the Deepgram Listen Flux API.
                 */
                struct Connected {
                    std::string type;
                    std::string request_id;
                    int sequence_id;

                    static Connected fromJson(const nlohmann::json& j) {
                        return Connected{
                            j.at("type").get<std::string>(),
                            j.at("request_id").get<std::string>(),
                            j.at("sequence_id").get<int>()
                        };
                    }
                };

                /**
                 * @brief Represents a TurnInfo event from the Deepgram Listen Flux API.
                 */
                struct TurnInfo {
                    std::string type;
                    std::string request_id;
                    int sequence_id;
                    std::string event;
                    int turn_index;
                    double audio_window_start;
                    double audio_window_end;
                    std::string transcript;
                    struct Word {
                        std::string word;
                        double confidence;

                        static Word fromJson(const nlohmann::json& j) {
                            return Word{
                                j.at("word").get<std::string>(),
                                j.at("confidence").get<double>()
                            };
                        }
                    };
                    std::vector<Word> words;
                    double end_of_turn_confidence;

                    static TurnInfo fromJson(const nlohmann::json& j) {
                        std::vector<Word> words;
                        if (j.contains("words")) {
                            for (const auto& word_json : j.at("words")) {
                                words.push_back(Word::fromJson(word_json));
                            }
                        }

                        return TurnInfo{
                            j.at("type").get<std::string>(),
                            j.at("request_id").get<std::string>(),
                            j.at("sequence_id").get<int>(),
                            j.at("event").get<std::string>(),
                            j.at("turn_index").get<int>(),
                            j.at("audio_window_start").get<double>(),
                            j.at("audio_window_end").get<double>(),
                            j.at("transcript").get<std::string>(),
                            words,
                            j.at("end_of_turn_confidence").get<double>()
                        };
                    }
                };

                struct FatalError {
                    std::string type;
                    int sequence_id;
                    std::string code;
                    std::string description;

                    static FatalError fromJson(const nlohmann::json& j) {
                        return FatalError{
                            j.at("type").get<std::string>(),
                            j.at("sequence_id").get<int>(),
                            j.at("code").get<std::string>(),
                            j.at("description").get<std::string>()
                        };
                    }
                };
            }

            // Forward declaration of the implementation class
            class ListenFluxClientImpl;

            /**
             * @brief Client for Deepgram's Listen Flux API.
             * This class provides methods to connect to the Listen Flux API, stream audio data, and handle incoming transcription events.
             */
            class DEEPGRAMPP_EXPORT ListenFluxClient {
                public:
                ListenFluxClient(const std::string& apiKey);
                ~ListenFluxClient();

                /**
                 * @brief Connects to the Deepgram Listen Flux API with the specified query parameters.
                 * @param params The query parameters for the connection.
                 * @return true if the connection was successful, false otherwise.
                 */
                bool connect(const FluxQueryParams& params);

                /**
                 * @brief Starts receiving events from the Deepgram Listen Flux API.
                 * This method should be called after a successful connection.
                 */
                void startReceiving();

                /**
                 * @brief Stops receiving events from the Deepgram Listen Flux API.
                 * This method can be used to gracefully shut down the receiving loop.
                 */
                void stopReceiving();

                /**
                 * @brief Streams audio data to the Deepgram Listen Flux API.
                 * @param audioData The audio data to stream.
                 * @param chunkSize The size of each audio chunk (default is 4096).
                 * @return true if the audio data was streamed successfully, false otherwise.
                 */
                bool streamAudio(const std::vector<uint8_t>& audioData, int chunkSize = 4096);

                /**
                 * @brief Sends a CloseStream control message to the Deepgram Listen Flux API.
                 * This indicates that no more audio data will be sent and the stream should be closed.
                 */
                void sendCloseStream();

                /**
                 * Callback function types for handling events.
                 */
                using OnTurnInfoCallback = std::function<void(const event::TurnInfo&)>;
                using OnConnectedCallback = std::function<void(const event::Connected&)>;
                using OnFatalErrorCallback = std::function<void(const event::FatalError&)>;

                /**
                 * @brief Sets the callback function to be invoked when a TurnInfo event is received.
                 * @param callback The callback function to handle TurnInfo events.
                 */
                void setOnTurnInfoCallback(OnTurnInfoCallback callback);

                /**
                 * @brief Sets the callback function to be invoked when a Connected event is received.
                 * @param callback The callback function to handle Connected events.
                 */
                void setOnConnectedCallback(OnConnectedCallback callback);

                /**
                 * @brief Sets the callback function to be invoked when a FatalError event is received.
                 * @param callback The callback function to handle FatalError events.
                 */
                void setOnFatalErrorCallback(OnFatalErrorCallback callback);

                private:
                std::unique_ptr<ListenFluxClientImpl> _fluxClientImpl;
                OnTurnInfoCallback _onTurnInfoCallback = [](const event::TurnInfo&) {};
                OnConnectedCallback _onConnectedCallback = [](const event::Connected&) {};
                OnFatalErrorCallback _onFatalErrorCallback = [](const event::FatalError&) {};
            };
        }
    }
}

#endif // DEEPGRAMPP_LISTEN_FLUX_HPP