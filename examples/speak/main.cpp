#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <functional>
#include <deepgrampp/deepgram.hpp>


int testSpeak(const char* apiKey) {
    try {
        deepgram::speak::SpeakWebsocketClient client("api.deepgram.com", apiKey);
        deepgram::speak::LiveSpeakConfig config({
            .model = deepgram::speak::models::featured::en::APOLLO,
            .sampleRate = 16000,
            .encoding = deepgram::speak::encoding::LINEAR_16
        });
        if(!client.connect(config)) {
            spdlog::error("Failed to connect to Deepgram.");
            return EXIT_FAILURE;
        }

        std::ofstream audioFile("/Users/fateh/Documents/GitHub/deepgrampp/test.raw", std::ios::binary);
        client.setSpeechResultCallback([&audioFile](const char *data, int size) {
            audioFile.write(data, size);
            audioFile.flush();
        });
        client.setSpeechControlResponseCallback([](const deepgram::speak::SpeakControlResponse &response) {
            spdlog::info("Received speech control response: {}", response.toString());
        });
        client.setSpeechCloseFrameCallback([](const deepgram::speak::SpeakCloseFrame &frame) {
            spdlog::info("Received speech close frame: {}", frame.toString());
        });
        client.setSpeechMetadataResponseCallback([](const deepgram::speak::MetadataResponse &response) {
            spdlog::info("Received metadata response: {}", response.toString());
        });

        client.startReceiving();

        client.speak("Hello, this is a sample speech to test Deepgram's capabilities. it will be processed and saved to a file.");
        client.sendFlushMessage();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error: {}", e.what());
        return EXIT_FAILURE;
    }
}

int main()
{
    constexpr const char *apiKey = "da3313d72d69f139a4d19f1b19fd0848ab22fdfa";
    testSpeak(apiKey);
    return 0;
}