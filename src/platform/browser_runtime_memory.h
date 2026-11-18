#pragma once

#include "browser_guest_memory.h"

#include <cstddef>
#include <cstdint>

namespace BrowserRuntimeMemory {

// Checked, browser-safe subset of WiiCompiled's guest-memory API.
// Raw guest pointers are intentionally omitted; translated code's flat 4 GiB
// pointer path needs a separate WebAssembly-compatible design.
class Adapter {
public:
    explicit Adapter(BrowserGuestMemory::Memory& memory) noexcept : memory_(memory) {}

    bool Contains(std::uint32_t address, std::size_t length = 1) const noexcept;
    bool Read8(std::uint32_t address, std::uint8_t& value) const noexcept;
    bool Read16(std::uint32_t address, std::uint16_t& value) const noexcept;
    bool Read32(std::uint32_t address, std::uint32_t& value) const noexcept;
    bool Read64(std::uint32_t address, std::uint64_t& value) const noexcept;
    bool ReadFloat32(std::uint32_t address, float& value) const noexcept;
    bool ReadFloat64(std::uint32_t address, double& value) const noexcept;

    bool Write8(std::uint32_t address, std::uint8_t value) noexcept;
    bool Write16(std::uint32_t address, std::uint16_t value) noexcept;
    bool Write32(std::uint32_t address, std::uint32_t value) noexcept;
    bool Write64(std::uint32_t address, std::uint64_t value) noexcept;
    bool WriteFloat32(std::uint32_t address, float value) noexcept;
    bool WriteFloat64(std::uint32_t address, double value) noexcept;

    bool CopyFromGuest(std::uint32_t address, void* destination, std::size_t length) const noexcept;
    bool CopyToGuest(std::uint32_t address, const void* source, std::size_t length) noexcept;

private:
    BrowserGuestMemory::Memory& memory_;
};

bool RunSmokeTest() noexcept;

} // namespace BrowserRuntimeMemory
