#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <functional>
#include <deepgrampp/deepgram.hpp>
#include <sstream>

int testSpeak(const char* apiKey) {
    try {
        deepgram::speak::SpeakWebsocketClient client(apiKey);
        deepgram::speak::LiveSpeakConfig config({
            .model = deepgram::speak::models::featured::en::THALIA,
            .sampleRate = 16000,
            .encoding = deepgram::speak::encoding::LINEAR_16
        });
        if(!client.connect(config)) {
            spdlog::error("Failed to connect to Deepgram.");
            return EXIT_FAILURE;
        }

        std::ofstream audioFile("audio-output.raw", std::ios::binary);
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

        for(int i=0;i<10;i++) {
            std::ostringstream os;
            os <<" hey " << i << " - " << "This is a test sentence for Deepgram's speech synthesis capabilities.";
            client.speak(os.str());
            //std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        //client.sendFlushMessage();
        //std::this_thread::sleep_for(std::chrono::seconds(3));
        client.speak("Yo, this is another sample sentence.");
        //client.sendFlushMessage();
        std::this_thread::sleep_for(std::chrono::seconds(200));
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
    constexpr const char *apiKey = "API_KEY";
    testSpeak(apiKey);
    return 0;
}