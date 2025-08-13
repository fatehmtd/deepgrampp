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

int main()
{
    try
    {
        // Read the audio file
        std::cout << "Reading audio file..." << std::endl;
        auto audioData = readAudioFile("/Users/fateh/Documents/recording.raw");

        if (audioData.empty())
        {
            std::cerr << "Error: Audio file is empty." << std::endl;
            return EXIT_FAILURE;
        }

        constexpr const char *apiKey = "da3313d72d69f139a4d19f1b19fd0848ab22fdfa";
        // Create and connect the client
        deepgram::listen::ListenWebsocketClient client("api.deepgram.com", apiKey);

        deepgram::listen::LiveTranscriptionOptions options({.model = deepgram::listen::models::nova_3::GENERAL,
                                                            .language = deepgram::listen::languages::nova_3::MULTILINGUAL,
                                                            .sampleRate = 16000,
                                                            .encoding = deepgram::listen::encoding::LINEAR_16,
                                                            .interimResults = true});

        if (!client.connect(options))
        {
            std::cerr << "Failed to connect to Deepgram." << std::endl;
            return EXIT_FAILURE;
        }

        client.setOnPartialTranscription([](const deepgram::listen::TranscriptionResult &result)
                                         {
            spdlog::info("Partial transcription: ");
            result.print(); });

        client.setOnFinalTranscription([&client](const deepgram::listen::TranscriptionResult &result)
                                       {
            spdlog::info("Final transcription: ");
            result.print(); });

        // Start receiving messages
        client.startReceiving();

        // Wait for connection to stabilize and receive metadata
        std::cout << "Waiting for metadata..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Stream the audio file
        if (!client.streamAudioFile(audioData))
        {
            std::cerr << "Failed to stream audio file." << std::endl;
            return EXIT_FAILURE;
        }

        // Wait for final transcription results
        std::cout << "Waiting for final results..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::cout << "Transcription complete!" << std::endl;
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}