
# deepgrampp

This is a community effort aimed at providing a C++ implementation of deepgram's Transcription and Speech Synthesis APIs.

It provides a simple and efficient way to integrate Deepgram's powerful speech recognition and synthesis capabilities into your C++ applications.

## Features

- Real-time speech recognition
- Text-to-speech synthesis
- Easy integration with existing C++ applications
- Support for multiple audio formats

## Usage

To use deepgrampp in your C++ project, follow these steps:

1. Include the necessary headers
2. Initialize the Deepgram client with your API key
3. Use the provided methods for transcription and synthesis

### Transcription

To transcribe audio using deepgrampp, use the following code snippet:

```cpp
#include <deepgrampp/deepgrampp.hpp>

int main() {
     // Create and connect the client
    deepgram::listen::ListenWebsocketClient client("api.deepgram.com", "API_KEY");

    deepgram::listen::LiveTranscriptionOptions options({.model = deepgram::listen::models::nova_3::GENERAL,
                                                        .language = deepgram::listen::languages::nova_3::MULTILINGUAL,
                                                        .sampleRate = 16000,
                                                        .encoding = deepgram::listen::encoding::LINEAR_16,
                                                        .interimResults = true});

    if (!client.connect(options))
    {
        spdlog::error("Failed to connect to Deepgram.");
        return EXIT_FAILURE;
    }

    client.setOnPartialTranscription([](const deepgram::listen::TranscriptionResult &result)
                                    {
                                        spdlog::info("Partial transcription: {}",
                                         result.channel.alternatives[0].transcript);
                                    });

    client.setOnFinalTranscription([&client](const deepgram::listen::TranscriptionResult &result)
                                    { 
                                        spdlog::info("Final transcription: {}",
                                         result.channel.alternatives[0].transcript);
                                    });

    // Start receiving messages
    client.startReceiving();

    // Wait for connection to stabilize and receive metadata
    spdlog::info("Waiting for metadata...");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // raw pcm audio
    std::vector<uint8_t> audioData = readAudioFile("path/to/audio/file.raw");

    // Stream the audio file
    if (!client.streamAudioFile(audioData))
    {
        spdlog::error("Failed to stream audio file.");
        return EXIT_FAILURE;
    }

    // Wait for final transcription results
    spdlog::info("Waiting for final results...");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    spdlog::info("Transcription complete!");
    client.close();
    return EXIT_SUCCESS;
}
```

### Speech Synthesis

To synthesize speech using deepgrampp, use the following code snippet:

```cpp
#include <deepgrampp/deepgrampp.h>

int main() {
    deepgram::DeepgramClient client("YOUR_API_KEY");

    // Synthesize speech from text
    auto response = client.synthesize("Hello, world!");

    if (response) {
        std::cout << "Synthesis successful: " << response->get_audio_url() << std::endl;
    } else {
        std::cerr << "Error: " << client.get_last_error() << std::endl;
    }

    return 0;
}
```

## License

This project is licensed under the Apache License - see the [LICENSE](LICENSE) file for details.
