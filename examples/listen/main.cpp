#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <functional>
#include <deepgrampp/deepgram.hpp>

std::vector<uint8_t> readAudioFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        throw std::runtime_error("Failed to open audio file: " + filePath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        throw std::runtime_error("Failed to read audio file");
    }

    std::cout << "Successfully read " << size << " bytes from audio file" << std::endl;
    return buffer;
}

int testListen(const char *apiKey, const std::string& audioFilePath)
{
    try
    {
        // Read the audio file
        spdlog::info("Reading audio file... {}", audioFilePath);
        auto audioData = readAudioFile(audioFilePath);

        if (audioData.empty())
        {
            spdlog::error("Error: Audio file is empty.");
            return EXIT_FAILURE;
        }

        // Create and connect the client
        deepgram::listen::ListenWebsocketClient client(apiKey);

        deepgram::listen::LiveTranscriptionOptions options;
        options.model = deepgram::listen::models::nova_3::GENERAL;
        options.language = deepgram::listen::languages::nova_3::MULTILINGUAL;
        options.sampleRate = 16000;
        options.encoding = deepgram::listen::encoding::LINEAR_16;
        options.interimResults = true;

        if (!client.connect(options))
        {
            spdlog::error("Failed to connect to Deepgram.");
            return EXIT_FAILURE;
        }

        client.setOnPartialTranscription([](const deepgram::listen::TranscriptionResult &result)
                                         { spdlog::info("Partial transcription: {}", result.channel.alternatives[0].transcript); });

        client.setOnFinalTranscription([&client](const deepgram::listen::TranscriptionResult &result)
                                       { spdlog::info("Final transcription: {}", result.channel.alternatives[0].transcript); });

        // Start receiving messages
        client.startReceiving();

        // Wait for connection to stabilize and receive metadata
        spdlog::info("Waiting for metadata...");
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Stream the audio file
        if (!client.streamAudio(audioData))
        {
            spdlog::error("Failed to stream audio file.");
            return EXIT_FAILURE;
        }

        // Wait for final transcription results
        spdlog::info("Waiting for final results...");
        std::this_thread::sleep_for(std::chrono::seconds(5));

        spdlog::info("Transcription complete!");
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
    testListen(apiKey, "sample_speech.raw");
    return 0;
}