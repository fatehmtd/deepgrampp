# Deepgrampp

**Deepgrampp** is an independent, community-driven C++ SDK for Deepgram's Speech-to-Text (transcription) and Text-to-Speech (synthesis) APIs. It offers real-time streaming capabilities, straightforward integration, and modern C++.

## Key Features

### Speech Recognition (STT)

- **Real-time transcription** via Deepgram WebSocket API with interim and final results
- **Flux API support** - Deepgram's latest streaming transcription technology
- **Multiple model support**: Nova-3 (latest), Nova-2, Nova-1, Base, Enhanced, and Whisper models
- **Comprehensive language support**: 10+ languages for Nova-3, 36+ languages for Nova-2
- **Advanced features**: Voice Activity Detection (VAD), speaker diarization, endpointing, utterance detection
- **Multiple audio formats**: LINEAR16, FLAC, Opus, AMR, G729, and more

### Text-to-Speech (TTS)

- **Real-time speech synthesis** via Deepgram WebSocket API
- **Aura 2 voices** - Latest generation high-quality voices (40+ English voices, 10+ Spanish voices)
- **Aura 1 voices** - Legacy voices still supported
- **Multiple accents**: American, British, Australian, Irish, Filipino, Mexican, Peninsular Spanish, Colombian
- **Codeswitching support** - Seamless switching between English and Spanish
- **Multiple audio formats**: LINEAR16, LINEAR32, FLAC, Opus, AAC, MP3, and more
- **Voice characteristics**: Professional, casual, energetic, calm, warm, confident voices optimized for different use cases

### Technical Features

- **Modern C++17 codebase** with comprehensive error handling
- **Logging via [spdlog](https://github.com/gabime/spdlog)** with colored output support
- **JSON parsing via [nlohmann/json](https://github.com/nlohmann/json)**
- **Cross-platform support** with CMake build system
- **Example applications** for all supported APIs
- **Memory-efficient streaming** with configurable chunk sizes

> **Note:**  
> This project uses [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) for networking, enabling robust and portable WebSocket and TCP connections for communication with Deepgram's APIs. There is a TODO item to provide support for alternative networking apis for a lighter integration.

## Available Models & Voices

### Speech-to-Text Models

| Model Family | Best For | Languages | Notes |
|--------------|----------|-----------|-------|
| **Nova-3** | Latest, highest accuracy | 10+ languages + multilingual | Recommended for new projects |
| **Nova-2** | Legacy but stable | 36+ languages + multilingual | Deprecated, use Nova-3 |
| **Flux** | Ultra-low latency streaming | English (more coming) | Latest streaming technology |
| **Whisper** | OpenAI compatibility | 30+ languages | Via Deepgram Cloud |
| **Enhanced** | Balanced accuracy/speed | 12+ languages | Good for production |
| **Base** | Cost-effective | 12+ languages | Budget-friendly option |

### Text-to-Speech Voices

#### Featured Aura 2 English Voices

- **THALIA** - Clear, confident, energetic (versatile)
- **ANDROMEDA** - Casual, expressive, comfortable (customer service)
- **HELENA** - Caring, natural, positive (friendly interactions)
- **APOLLO** - Confident, comfortable (professional presentations)
- **ARCAS** - Natural, smooth, clear (general purpose)

#### Featured Aura 2 Spanish Voices

- **CELESTE** - Clear, energetic, positive (Colombian accent)
- **ESTRELLA** - Approachable, natural, calm (Mexican accent)
- **NESTOR** - Calm, professional (Peninsular accent)

> **Voice Selection Tip**: The SDK includes 40+ English and 10+ Spanish voices optimized for different use cases (customer service, IVR, advertising, storytelling, casual conversation). See `speak.hpp` for the complete catalog.

## Getting Started

### Prerequisites

- C++17 compiler
- [CMake](https://cmake.org/) >= 3.16
- [Boost](https://www.boost.org/) (system, thread)
- [OpenSSL](https://www.openssl.org/)

### Build Instructions

```sh
git clone https://github.com/fatehmtd/deepgrampp.git
cd deepgrampp
cmake -S . -B build
cmake --build build
```

This will build the core library and example applications.

## Example Usage

### Speech Synthesis (Text-to-Speech)

See [examples/speak/main.cpp](examples/speak/main.cpp):

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    // Create client with your API key
    deepgram::speak::SpeakWebsocketClient client("YOUR_API_KEY");
    
    // Configure with Aura 2 voice
    deepgram::speak::LiveSpeakConfig config;
    config.model = deepgram::speak::models::featured::en::THALIA; // Clear, confident voice
    config.encoding = deepgram::speak::encoding::LINEAR_16;
    config.sampleRate = 16000;
    
    if (!client.connect(config)) return 1;

    // Set up callbacks for audio data and events
    std::ofstream audioFile("output.raw", std::ios::binary);
    client.setSpeechResultCallback([&audioFile](const char* data, int size) {
        audioFile.write(data, size);
    });
    
    client.setSpeechControlResponseCallback([](const deepgram::speak::SpeakControlResponse& response) {
        spdlog::info("Control response: {}", response.toString());
    });

    client.startReceiving();
    client.speak("Hello from Deepgram's advanced text-to-speech!");
    client.sendFlushMessage();
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::seconds(3));
    client.close();
    return 0;
}
```

### Speech Recognition (Speech-to-Text)

#### Standard WebSocket API

See [examples/listen/main.cpp](examples/listen/main.cpp):

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    // Create client with your API key
    deepgram::listen::ListenWebsocketClient client("YOUR_API_KEY");

    // Configure transcription options
    deepgram::listen::LiveTranscriptionOptions options;
    options.model = deepgram::listen::models::nova_3::GENERAL;
    options.language = deepgram::listen::languages::nova_3::MULTILINGUAL;
    options.sampleRate = 16000;
    options.encoding = deepgram::listen::encoding::LINEAR_16;
    options.interimResults = true;
    options.vadEvents = true;

    if (!client.connect(options)) return 1;

    // Set up transcription callbacks
    client.setOnPartialTranscription([](const deepgram::listen::TranscriptionResult& result) {
        spdlog::info("Partial: {}", result.getBestTranscript());
    });
    
    client.setOnFinalTranscription([](const deepgram::listen::TranscriptionResult& result) {
        spdlog::info("Final: {}", result.getBestTranscript());
    });

    client.setOnSpeechStarted([](const deepgram::listen::SpeechStarted&) {
        spdlog::info("Speech started");
    });

    // Read audio data from file or microphone
    std::vector<uint8_t> audioData = readAudioFile("sample.raw");
    
    client.startReceiving();
    client.streamAudio(audioData);
    
    // Wait for transcription completion
    std::this_thread::sleep_for(std::chrono::seconds(10));
    client.close();
    return 0;
}
```

#### Flux API (Latest Streaming Technology)

See [examples/listen-flux/main.cpp](examples/listen-flux/main.cpp):

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    // Create Flux client with your API key
    deepgram::listen::flux::ListenFluxClient client("YOUR_API_KEY");

    // Configure Flux parameters
    deepgram::listen::flux::FluxQueryParams options;
    options.model = deepgram::listen::flux::model::FLUX_GENERAL_EN;
    options.encoding = deepgram::listen::flux::encoding::LINEAR16;
    options.sample_rate = 16000;

    if (!client.connect(options)) return 1;

    // Set up Flux event callbacks
    client.setOnConnectedCallback([](const deepgram::listen::flux::event::Connected& event) {
        spdlog::info("Connected to Flux API - Request ID: {}", event.request_id);
    });

    client.setOnTurnInfoCallback([](const deepgram::listen::flux::event::TurnInfo& turnInfo) {
        spdlog::info("Transcript: {}", turnInfo.transcript);
        spdlog::info("Confidence: {:.2f}", turnInfo.end_of_turn_confidence);
        
        // Display word-level confidence with color coding
        for (const auto& word : turnInfo.words) {
            std::string color = (word.confidence >= 0.8) ? "🟢" : 
                               (word.confidence >= 0.6) ? "🟡" : "🔴";
            spdlog::info("  {} {} ({:.2f})", color, word.word, word.confidence);
        }
    });

    // Read and stream audio data
    std::vector<uint8_t> audioData = readAudioFile("sample.raw");
    client.startReceiving();
    client.streamAudio(audioData);
    
    // Wait for results
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
```

## Project Structure

```text
deepgrampp/
├── CMakeLists.txt                 # Library build configuration
├── include/deepgrampp/            # Public headers
│   ├── deepgram.hpp              # Main include file
│   ├── listen.hpp                # STT data structures and models
│   ├── listen-ws.hpp             # WebSocket STT client
│   ├── listen-flux.hpp           # Flux API STT client
│   ├── speak.hpp                 # TTS data structures and models
│   └── speak-ws.hpp              # WebSocket TTS client
├── src/                          # Implementation files
│   ├── listen-ws.cpp
│   ├── listen-flux.cpp
│   ├── speak-ws.cpp
│   └── impl/                     # Platform-specific implementations
examples/
├── listen/                       # Standard STT WebSocket example
├── listen-flux/                  # Flux API STT example
└── speak/                        # TTS WebSocket example
```

## License

Apache License 2.0 — see [LICENSE](LICENSE).

## Contributing

Pull requests and issues are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) if available.

## Roadmap & TODO

### ✅ Completed Features

- [x] **WebSocket Streaming APIs** - Both STT and TTS with real-time streaming
- [x] **Flux API Support** - Latest Deepgram streaming transcription technology
- [x] **Comprehensive Model Support** - Nova-3, Nova-2, Nova-1, Base, Enhanced, Whisper models
- [x] **Advanced Voice Catalog** - 40+ Aura 2 English voices, 10+ Spanish voices with codeswitching
- [x] **Multiple Audio Formats** - LINEAR16, FLAC, Opus, AMR, G729, AAC, MP3, and more
- [x] **Advanced STT Features** - VAD, diarization, endpointing, interim results, utterance detection
- [x] **Comprehensive Language Support** - 36+ languages with multilingual detection
- [x] **Robust Error Handling** - Detailed error reporting and connection management  
- [x] **Logging Framework** - spdlog integration with colored output
- [x] **JSON Processing** - nlohmann/json integration
- [x] **CMake Build System** - Cross-platform build configuration
- [x] **Example Applications** - Complete examples for all supported APIs
- [x] **Memory-Efficient Streaming** - Configurable chunk sizes and optimized data handling

### 🚧 In Progress

- [ ] **REST API Support** - Add support for batch transcription and synthesis
- [ ] **Alternative Networking** - Reduce Boost.Asio dependency with lighter alternatives
- [ ] **Intelligence Layer** - Support for Deepgram's advanced AI features
- [ ] **Audio Processing Utilities** - Built-in audio format conversion and preprocessing

### 🔮 Future Enhancements

- [ ] **Unit & Integration Tests** - Comprehensive test coverage
- [ ] **CMake Package Config** - Find package support for easier integration
- [ ] **API Documentation** - Complete API reference documentation
- [ ] **Performance Optimizations** - Benchmarking and optimization for high-throughput scenarios
- [ ] **Multi-platform Packaging** - Pre-built packages for major platforms

> [!WARNING]
> The text to speech struggles with relatively long sentences even when a flush command is sent right after. The workaround that I found for now, is to wait for about 1 second then send the subsequent text to synthesize. I am actively investigating this issue.

## Acknowledgements

This is an unofficial API developed for use in my own projects and shared with the community. I am not affiliated with Deepgram in any way.