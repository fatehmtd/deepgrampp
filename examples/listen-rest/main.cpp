#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
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

void printResult(const deepgram::listen::PrerecordedTranscriptionResult& result)
{
    if (!result)
    {
        spdlog::error("Transcription failed (HTTP {}): {}", result.statusCode, result.errorMessage);
        return;
    }

    for (const auto& channel : result.response.channels)
    {
        if (!channel.alternatives.empty())
        {
            spdlog::info("Transcript: {}", channel.alternatives[0].transcript);
        }
    }
}

int testListenRest(const char* apiKey, const std::string& audioFilePath)
{
    try
    {
        deepgram::listen::ListenRestClient client(apiKey);

        deepgram::listen::LiveTranscriptionOptions options;
        options.model = deepgram::listen::models::nova_3::GENERAL;
        options.language = deepgram::listen::languages::nova_3::MULTILINGUAL;
        options.sampleRate = 16000;
        options.encoding = deepgram::listen::encoding::LINEAR_16;
        options.channels = 1;
        options.smartFormat = true;

        // Option 1: let the client read the audio file straight from disk.
        spdlog::info("Transcribing {} via transcribeFile()...", audioFilePath);
        auto fileResult = client.transcribeFile(audioFilePath, "audio/L16", options);
        printResult(fileResult);

        // Option 2: transcribe an in-memory buffer (e.g. audio captured elsewhere).
        spdlog::info("Transcribing the same audio via transcribeBuffer()...");
        auto audioData = readAudioFile(audioFilePath);
        auto bufferResult = client.transcribeBuffer(audioData, "audio/L16", options);
        printResult(bufferResult);

        return (fileResult.success && bufferResult.success) ? EXIT_SUCCESS : EXIT_FAILURE;
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
    return testListenRest(apiKey, "sample_speech.raw");
}
