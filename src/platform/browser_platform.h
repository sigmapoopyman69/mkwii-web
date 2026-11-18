#pragma once

#include <cstdint>

namespace BrowserPlatform {

// Browser services needed before the translated game runtime can be hosted.
enum Capability : std::uint32_t {
    WebGPU = 1u << 0,
    Gamepad = 1u << 1,
    WebAudio = 1u << 2,
    IndexedDB = 1u << 3,
    SharedMemory = 1u << 4,
    Fullscreen = 1u << 5
};

std::uint32_t GetCapabilities() noexcept;
double NowMilliseconds() noexcept;

} // namespace BrowserPlatform
