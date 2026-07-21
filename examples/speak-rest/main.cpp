#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <functional>
#include <deepgrampp/deepgram.hpp>
#include <cstdlib>

int testSpeakRest(const char* apiKey) {
    try {
        deepgram::speak::SpeakRestClient client(apiKey);

        deepgram::speak::LiveSpeakConfig config;
        config.model = deepgram::speak::models::featured::en::ARCAS;
        config.encoding = deepgram::speak::encoding::LINEAR_16;
        config.sampleRate = 16000;
        config.container = "wav";

        spdlog::info("Requesting speech synthesis...");
        auto result = client.speak("This is a test sentence for Deepgram's speech synthesis capabilities.", config);

        if (!result) {
            spdlog::error("Speech synthesis failed (HTTP {}): {}", result.statusCode, result.errorMessage);
            return EXIT_FAILURE;
        }

        if (result.requestId) {
            spdlog::info("Request ID: {}", *result.requestId);
        }
        if (result.modelName) {
            spdlog::info("Model: {}", *result.modelName);
        }
        spdlog::info("Content-Type: {}", result.contentType);
        spdlog::info("Received {} bytes of audio", result.audio.size());

        // raw file to write into, can be opened with Audacity (https://www.audacityteam.org/)
        std::ofstream audioFile("audio-output.wav", std::ios::binary);
        audioFile.write(reinterpret_cast<const char*>(result.audio.data()), result.audio.size());
        audioFile.close();
        spdlog::info("Audio written to audio-output.wav");

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
    if (!apiKey) {
        spdlog::error("DEEPGRAM_API_KEY environment variable is not set.");
        return EXIT_FAILURE;
    }
    return testSpeakRest(apiKey);
}
