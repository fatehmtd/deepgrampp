# deepgrampp

**deepgrampp** is an independent, community-driven C++ SDK for Deepgram’s Speech-to-Text (transcription) and Text-to-Speech (synthesis) APIs. It offers real-time streaming capabilities, straightforward integration, and modern C++.


## Features

- Real-time speech recognition (transcription) via Deepgram WebSocket API
- Text-to-speech synthesis with streaming audio output
- Support for multiple models, languages, and audio formats
- Example applications for both transcription and synthesis
- Logging via [spdlog](https://github.com/gabime/spdlog)
- JSON parsing via [nlohmann/json](https://github.com/nlohmann/json)
- Modern C++17 codebase

> **Note:**  
> This project uses [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) for networking, enabling robust and portable WebSocket and TCP connections for communication with Deepgram’s APIs. There is a TODO item to provide support for alternative networking apis for a lighter integration.
>

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

### Example Usage

#### Speech Synthesis

See [examples/speak/main.cpp](examples/speak/main.cpp):

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    deepgram::speak::SpeakWebsocketClient client("api.deepgram.com", "YOUR_API_KEY");
    deepgram::speak::LiveSpeakConfig config({
        .model = deepgram::speak::models::featured::en::APOLLO,
        .sampleRate = 16000,
        .encoding = deepgram::speak::encoding::LINEAR_16
    });
    if (!client.connect(config)) return 1;
    client.setSpeechResultCallback([](const char *data, int size) {
        // handle audio data
    });
    client.startReceiving();
    client.speak("Hello, world!");
    client.sendFlushMessage();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    client.close();
    return 0;
}
```

#### Speech Recognition

See [examples/listen/main.cpp](examples/listen/main.cpp):

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    deepgram::listen::ListenWebsocketClient client("api.deepgram.com", "YOUR_API_KEY");
    deepgram::listen::LiveTranscriptionOptions options({
        .model = deepgram::listen::models::nova_3::GENERAL,
        .language = deepgram::listen::languages::nova_3::MULTILINGUAL,
        .sampleRate = 16000,
        .encoding = deepgram::listen::encoding::LINEAR_16,
        .interimResults = true
    });
    if (!client.connect(options)) return 1;
    client.setOnPartialTranscription([](const deepgram::listen::TranscriptionResult &result) {
        // handle partial transcript
    });
    client.setOnFinalTranscription([](const deepgram::listen::TranscriptionResult &result) {
        // handle final transcript
    });
    client.startReceiving();
    // stream audio data...
    client.close();
    return 0;
}
``` 

## Project Structure

- `deepgrampp/` — Core library source and headers
- `examples/` — Example applications (`listen` and `speak`)
- `CMakeLists.txt` — Top-level build configuration

## License

Apache License 2.0 — see [LICENSE](LICENSE).

## Contributing

Pull requests and issues are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) if available.

## TODO

- [x] Support Deepgram WebSocket streaming API
- [x] Provide a list of available models for Transcription (See [listen.hpp](deepgrampp/include/deepgrampp/listen.hpp)) and Synthesis (See [speak.hpp](deepgrampp/include/deepgrampp/speak.hpp))
- [x] Logging via spdlog
- [x] JSON parsing via nlohmann/json
- [x] CMake build system
- [x] Example applications for transcription and synthesis
- [ ] Provide alternatives to **Boost.Asio** for WebSocket streaming
- [ ] Support Deepgram REST API in addition to WebSocket streaming
- [ ] Improve error handling and reporting
- [ ] Add unit and integration tests
- [ ] Provide CMake package config for easier integration
- [ ] Document all public APIs
- [ ] Support additional Deepgram features (mainly the intelligence layer)

## Acknowledgements

This is an unofficial API developed for use in my own projects and shared with the community. I am not affiliated with Deepgram in any way.