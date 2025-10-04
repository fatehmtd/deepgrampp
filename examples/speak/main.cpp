#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <functional>
#include <deepgrampp/deepgram.hpp>
#include <sstream>
#include <cstdlib>

int testSpeak(const char* apiKey) {
    spdlog::info("Starting Speak test.... key: {}", apiKey);
    try {
        deepgram::speak::SpeakWebsocketClient client(apiKey);
        deepgram::speak::LiveSpeakConfig config;
        config.model = deepgram::speak::models::featured::en::ARCAS;
        config.encoding = deepgram::speak::encoding::LINEAR_16;
        config.sampleRate = 16000;
        if (!client.connect(config)) {
            spdlog::error("Failed to connect to Deepgram.");
            return EXIT_FAILURE;
        }

        std::atomic<bool> isRunning = true;
        // raw file to write into, can be opened with Audacity (https://www.audacityteam.org/)
        std::ofstream audioFile("audio-output.raw", std::ios::binary);
        client.setSpeechResultCallback([&audioFile](const char* data, int size) {
            audioFile.write(data, size);
            audioFile.flush();
            });
        client.setSpeechControlResponseCallback([](const deepgram::speak::SpeakControlResponse& response) {
            spdlog::info("Received speech control response: {}", response.toString());
            });
        client.setSpeechCloseFrameCallback([&isRunning](const deepgram::speak::SpeakCloseFrame& frame) {
            spdlog::info("Received speech close frame: {}", frame.toString());
            isRunning = false;
            });
        client.setSpeechMetadataResponseCallback([](const deepgram::speak::MetadataResponse& response) {
            spdlog::info("Received metadata response: {}", response.toString());
            });

        client.setSpeechErrorCallback([&isRunning](const std::string& errorMessage) {
            spdlog::error("Received speech error: {}", errorMessage);
            isRunning = false;
            });

        client.setSpeechDisconnectedCallback([&isRunning]() {
            spdlog::warn("WebSocket disconnected.");
            isRunning = false;
            });

        client.setSpeechStartedCallback([]() {
            spdlog::info("Speech started.");
        });

        client.setSpeechEndedCallback([&isRunning]() {
            spdlog::info("Speech ended.");
            isRunning = false;
        });

        client.startReceiving();

        client.speak("This is a test sentence for Deepgram's speech synthesis capabilities.");
        client.sendFlushMessage();

        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }

        client.close();
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
    testSpeak(apiKey);
    return 0;
}