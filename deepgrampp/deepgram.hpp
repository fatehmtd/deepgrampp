#pragma once

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

namespace deepgram
{
    namespace control
    {
        /**
         * Control messages for the Deepgram WebSocket API.
         *
         * @note These messages are used to control the WebSocket connection and stream.
         */
        constexpr const char* CLOSE_MESSAGE = R"({"type": "CloseStream"})";
        constexpr const char* KEEPALIVE_MESSAGE = R"({"type": "KeepAlive"})";
        constexpr const char* FINALIZE_MESSAGE = R"({"type": "Finalize"})";
    }
} // namespace deepgram