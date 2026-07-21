# Deepgrampp

An unofficial C++17 SDK for [Deepgram](https://deepgram.com/)'s Speech-to-Text and Text-to-Speech APIs: WebSocket streaming, the Flux streaming API, and the batch REST endpoints.

Not affiliated with Deepgram — built for personal projects and shared with the community.

## Requirements

- C++17 compiler
- CMake >= 3.16
- OpenSSL development headers, only if libcurl/libwebsockets need to be built from source (see below) — `libssl-dev`/`openssl-devel` on Linux, `brew install openssl@3` on macOS. If CMake can't find it, pass `-DOPENSSL_ROOT_DIR=...` or set `CMAKE_PREFIX_PATH`.

## Build

```sh
git clone https://github.com/fatehmtd/deepgrampp.git
cd deepgrampp
cmake -S . -B build
cmake --build build
```

Dependencies (spdlog, nlohmann/json, libcurl, libwebsockets) are looked up on the system first via `find_package`, and only fetched and built from source with `FetchContent` if missing — the first configure can take a few minutes in that case.

## Clients

| Client | API | Style |
|---|---|---|
| `ListenWebsocketClient` | `/v1/listen` streaming | Connect once, stream audio, get callbacks |
| `ListenFluxClient` | Flux streaming | Same shape as above, Deepgram's newer streaming model |
| `ListenRestClient` | `/v1/listen` batch | One blocking call, from a URL, buffer, or file |
| `SpeakWebsocketClient` | `/v1/speak` streaming | Connect once, send text, get audio callbacks |
| `SpeakRestClient` | `/v1/speak` batch | One blocking call, returns the full audio buffer |

WebSocket clients sit behind a small `deepgram::transport::IWebSocketTransport` interface (implemented by `LwsWebSocketTransport`, on top of libwebsockets); REST clients sit behind `IHttpTransport` (implemented by `CurlHttpTransport`, on top of libcurl). Both constructors accept the transport as an optional argument, so you can substitute a fake for testing.

## Usage

### Streaming transcription

See [examples/listen/main.cpp](examples/listen/main.cpp).

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    deepgram::listen::ListenWebsocketClient client("YOUR_API_KEY");

    deepgram::listen::LiveTranscriptionOptions options;
    options.model = deepgram::listen::models::nova_3::GENERAL;
    options.language = deepgram::listen::languages::nova_3::MULTILINGUAL;
    options.sampleRate = 16000;
    options.encoding = deepgram::listen::encoding::LINEAR_16;
    options.interimResults = true;

    if (!client.connect(options)) return 1;

    client.setOnFinalTranscription([](const deepgram::listen::TranscriptionResult& result) {
        spdlog::info("Final: {}", result.getBestTranscript());
    });

    std::vector<uint8_t> audioData = readAudioFile("sample.raw");
    client.streamAudio(audioData);

    std::this_thread::sleep_for(std::chrono::seconds(10));
    client.close();
    return 0;
}
```

### Flux streaming

See [examples/listen-flux/main.cpp](examples/listen-flux/main.cpp).

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    deepgram::listen::flux::ListenFluxClient client("YOUR_API_KEY");

    deepgram::listen::flux::FluxQueryParams options;
    options.model = deepgram::listen::flux::model::FLUX_GENERAL_EN;
    options.encoding = deepgram::listen::flux::encoding::LINEAR16;
    options.sample_rate = 16000;

    if (!client.connect(options)) return 1;

    client.setOnTurnInfoCallback([](const deepgram::listen::flux::event::TurnInfo& turnInfo) {
        spdlog::info("Transcript: {}", turnInfo.transcript);
    });

    std::vector<uint8_t> audioData = readAudioFile("sample.raw");
    client.streamAudio(audioData);

    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
```

### Batch transcription (REST)

See [examples/listen-rest/main.cpp](examples/listen-rest/main.cpp).

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    deepgram::listen::ListenRestClient client("YOUR_API_KEY");

    deepgram::listen::LiveTranscriptionOptions options;
    options.model = deepgram::listen::models::nova_3::GENERAL;
    options.smartFormat = true;

    // Also available: transcribeBuffer(audioData, contentType, options)
    // and transcribeUrl(url, options) for a remotely hosted file.
    auto result = client.transcribeFile("sample.wav", "audio/wav", options);
    if (!result) {
        spdlog::error("Transcription failed: {}", result.errorMessage);
        return 1;
    }

    spdlog::info("Transcript: {}", result.response.channels[0].alternatives[0].transcript);
    return 0;
}
```

Note: `options.sampleRate`/`options.encoding`/`options.channels` describe headerless raw PCM. Leave them as-is for raw audio (as in the example above); if the file/buffer/URL already carries its own container (a real `.wav`/`.mp3`), Deepgram auto-detects it, and setting these will make it misread the container as raw PCM.

### Streaming speech synthesis

See [examples/speak/main.cpp](examples/speak/main.cpp).

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    deepgram::speak::SpeakWebsocketClient client("YOUR_API_KEY");

    deepgram::speak::LiveSpeakConfig config;
    config.model = deepgram::speak::models::featured::en::THALIA;
    config.encoding = deepgram::speak::encoding::LINEAR_16;
    config.sampleRate = 16000;

    if (!client.connect(config)) return 1;

    std::ofstream audioFile("output.raw", std::ios::binary);
    client.setSpeechResultCallback([&audioFile](const char* data, int size) {
        audioFile.write(data, size);
    });

    client.speak("Hello from Deepgram's text-to-speech!");
    client.sendFlushMessage();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    client.close();
    return 0;
}
```

### Batch speech synthesis (REST)

See [examples/speak-rest/main.cpp](examples/speak-rest/main.cpp).

```cpp
#include <deepgrampp/deepgram.hpp>

int main() {
    deepgram::speak::SpeakRestClient client("YOUR_API_KEY");

    deepgram::speak::LiveSpeakConfig config;
    config.model = deepgram::speak::models::featured::en::ARCAS;
    config.encoding = deepgram::speak::encoding::LINEAR_16;
    config.container = "wav";

    auto result = client.speak("Hello from Deepgram's batch text-to-speech!", config);
    if (!result) {
        spdlog::error("Synthesis failed: {}", result.errorMessage);
        return 1;
    }

    std::ofstream audioFile("output.wav", std::ios::binary);
    audioFile.write(reinterpret_cast<const char*>(result.audio.data()), result.audio.size());
    return 0;
}
```

## Models and voices

| Model family | Notes |
|---|---|
| Nova-3 | Latest, recommended for new projects. 10+ languages plus multilingual mode. |
| Nova-2 | Deprecated, use Nova-3. 36+ languages. |
| Flux | Streaming-only, ultra-low latency. English for now. |
| Whisper | Via Deepgram Cloud, 30+ languages. |
| Enhanced / Base | Older, still supported. |

For TTS, `speak.hpp` has the full voice catalog (40+ English voices across the Aura 2/Aura 1 families, 10+ Spanish voices with codeswitching support). `models::featured::en`/`models::featured::es` are a curated shortlist if you don't want to pick manually.

Supported audio encodings vary by endpoint (raw PCM at various bit depths, FLAC, Opus, AAC, MP3, mu-law/a-law, etc.) — see `encoding` in `listen.hpp`/`speak.hpp`.

## Project layout

```text
deepgrampp/
├── CMakeLists.txt
├── include/deepgrampp/
│   ├── deepgram.hpp        # umbrella include
│   ├── listen.hpp          # STT types/options, shared by all listen clients
│   ├── listen-ws.hpp       # WebSocket streaming STT
│   ├── listen-rest.hpp     # batch STT
│   ├── listen-flux.hpp     # Flux streaming STT
│   ├── speak.hpp           # TTS types/options, shared by all speak clients
│   ├── speak-ws.hpp        # WebSocket streaming TTS
│   └── speak-rest.hpp      # batch TTS
├── src/                    # client implementations
│   └── impl/               # transport-backed impl classes (not installed)
└── transport/               # IWebSocketTransport/IHttpTransport + their implementations
examples/
├── listen/                 # WebSocket STT
├── listen-flux/             # Flux STT
├── listen-rest/             # batch STT
├── speak/                   # WebSocket TTS
└── speak-rest/               # batch TTS
```

## Known limitations

- TTS struggles with long sentences even right after a flush; waiting ~1s before sending the next sentence works around it. Under investigation.
- No automated test suite yet — the transport interfaces exist partly to make one possible (inject a fake transport in place of curl/libwebsockets), but nothing is wired up.
- No CMake package config (`find_package(deepgrampp)`) yet; consume it via `add_subdirectory` or your own install rules.

## License

Apache License 2.0 — see [LICENSE](LICENSE).

## Contributing

Issues and pull requests welcome.
