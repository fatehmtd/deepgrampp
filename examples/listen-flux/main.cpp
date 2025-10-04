#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <functional>
#include <deepgrampp/deepgram.hpp>
#include <cstdlib>

std::vector<uint8_t> readAudioFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        throw std::runtime_error("Failed to open audio file: " + filePath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
        throw std::runtime_error("Failed to read audio file");
    }

    std::cout << "Successfully read " << size << " bytes from audio file" << std::endl;
    return buffer;
}

std::string getColoredWord(const std::string& word, double confidence) {
    if (confidence >= 0.8) {
        return fmt::format("\033[32m{}\033[0m", word); // Green for high confidence
    } else if (confidence >= 0.6) {
        return fmt::format("\033[33m{}\033[0m", word); // Yellow for medium confidence
    } else {
        return fmt::format("\033[31m{}\033[0m", word); // Red for low confidence
    }
}

int testListen(const char* apiKey, const std::string& audioFilePath)
{
    try
    {
        // setup console logger with color support
        auto console = spdlog::stdout_color_mt("console");
        spdlog::set_default_logger(console);

        // Read the audio file
        spdlog::info("Reading audio file... {}", audioFilePath);
        auto audioData = readAudioFile(audioFilePath);

        if (audioData.empty())
        {
            spdlog::error("Error: Audio file is empty.");
            return EXIT_FAILURE;
        }

        // Create and connect the client
        deepgram::listen::flux::ListenFluxClient client(apiKey);

        deepgram::listen::flux::FluxQueryParams options;
        // currently only flux-general-en is supported
        options.model = deepgram::listen::flux::model::FLUX_GENERAL_EN;
        // currently only linear16 is supported
        options.encoding = deepgram::listen::flux::encoding::LINEAR16;
        options.sample_rate = 16000;

        if (!client.connect(options))
        {
            spdlog::error("Failed to connect to Deepgram.");
            return EXIT_FAILURE;
        }

        client.setOnConnectedCallback([](const deepgram::listen::flux::event::Connected& connected) {
            spdlog::info("Connected to Deepgram Listen Flux API");
            spdlog::info("  Type: {}", connected.type);
            spdlog::info("  Request ID: {}", connected.request_id);
            spdlog::info("  Sequence ID: {}", connected.sequence_id);
        });

        client.setOnTurnInfoCallback([](const deepgram::listen::flux::event::TurnInfo& turnInfo) {
            spdlog::info("=================================================================================");
            // print the turn info details
            spdlog::info("  Type: {}", turnInfo.type);
            spdlog::info("  Request ID: {}", turnInfo.request_id);
            spdlog::info("  Sequence ID: {}", turnInfo.sequence_id);
            spdlog::info("  Event: {}", turnInfo.event);
            spdlog::info("  Turn Index: {}", turnInfo.turn_index);
            spdlog::info("  Audio Window: {} - {}", turnInfo.audio_window_start, turnInfo.audio_window_end);
            spdlog::info("  Transcript: {}", turnInfo.transcript);
            spdlog::info("  Words:");
            for (const auto& word : turnInfo.words) {
                std::string coloredWord = getColoredWord(word.word, word.confidence);
                spdlog::info("    {} (confidence: {:.2f})", coloredWord, word.confidence);
            }
            spdlog::info("  End of Turn Confidence: {}", turnInfo.end_of_turn_confidence);
        });

        // Start receiving messages
        client.startReceiving();

        // Wait for connection to stabilize
        std::this_thread::sleep_for(std::chrono::seconds(2));

        for (int i = 0; i < 2; i++) {
            spdlog::info("Iteration: {} ****************************************************************************", i);
            // Stream the audio file
            if (!client.streamAudio(audioData))
            {
                spdlog::error("Failed to stream audio file.");
                return EXIT_FAILURE;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Wait for final transcription results
        spdlog::info("Waiting for final results...");
        std::this_thread::sleep_for(std::chrono::seconds(30));

        spdlog::info("Transcription complete!");
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Error: {}", e.what());
        return EXIT_FAILURE;
    }
}

int main()
{
    const char* apiKey = std::getenv("DEEPGRAM_API_KEY");
    spdlog::info("Using Deepgram API Key: {}", apiKey);
    testListen(apiKey, "sample_speech.raw");
    return 0;
}