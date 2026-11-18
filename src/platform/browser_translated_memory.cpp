#include "browser_translated_memory.h"

#include <cstring>

namespace {
BrowserGuestMemory::Memory g_memory;
BrowserRuntimeMemory::Adapter g_adapter(g_memory);
bool g_fault = false;

template <typename T, typename Read>
T ReadOrZero(std::uint32_t address, Read read) {
    T value{};
    if (!read(address, value)) {
        g_fault = true;
        return T{};
    }
    return value;
}

template <typename T, typename Write>
void WriteOrFault(std::uint32_t address, T value, Write write) {
    if (!write(address, value)) g_fault = true;
}
} // namespace

namespace BrowserTranslatedMemory {
BrowserGuestMemory::Memory& Backing() noexcept { return g_memory; }
BrowserRuntimeMemory::Adapter& Adapter() noexcept { return g_adapter; }
void ClearFault() noexcept { g_fault = false; }
bool HasFault() noexcept { return g_fault; }

bool RunSmokeTest() noexcept {
    g_memory.Reset();
    ClearFault();

    // Representative emitted C++ block: store a guest value, reload it through
    // the same helper ABI, then exercise a second-width access and alias.
    MemoryInline::FlatWrite32(0x80008000u, 0x57454238u);
    if (HasFault()) return false;
    if (MemoryInline::FlatRead32(0x00008000u) != 0x57454238u || HasFault()) return false;

    MemoryInline::FlatWrite64(0x90009000u, 0x0123456789ABCDEFull);
    if (HasFault()) return false;
    if (MemoryInline::FlatRead64(0x10009000u) != 0x0123456789ABCDEFull || HasFault()) return false;

    MemoryInline::FlatWriteFloat32(0x8000A000u, 1.5);
    if (HasFault()) return false;
    if (MemoryInline::FlatReadFloat32(0xC000A000u) != 1.5f || HasFault()) return false;

    // Invalid addresses must latch a fault instead of silently reading zero.
    (void)MemoryInline::FlatRead32(0x70000000u);
    if (!HasFault()) return false;
    ClearFault();
    return !HasFault();
}
} // namespace BrowserTranslatedMemory

namespace MemoryInline {
std::uint8_t FlatRead8(std::uint32_t a) {
    return ReadOrZero<std::uint8_t>(a, [](auto x, auto& v) { return BrowserTranslatedMemory::Adapter().Read8(x, v); });
}
std::uint16_t FlatRead16(std::uint32_t a) {
    return ReadOrZero<std::uint16_t>(a, [](auto x, auto& v) { return BrowserTranslatedMemory::Adapter().Read16(x, v); });
}
std::uint32_t FlatRead32(std::uint32_t a) {
    return ReadOrZero<std::uint32_t>(a, [](auto x, auto& v) { return BrowserTranslatedMemory::Adapter().Read32(x, v); });
}
std::uint64_t FlatRead64(std::uint32_t a) {
    return ReadOrZero<std::uint64_t>(a, [](auto x, auto& v) { return BrowserTranslatedMemory::Adapter().Read64(x, v); });
}
float FlatReadFloat32(std::uint32_t a) {
    return ReadOrZero<float>(a, [](auto x, auto& v) { return BrowserTranslatedMemory::Adapter().ReadFloat32(x, v); });
}
double FlatReadFloat64(std::uint32_t a) {
    return ReadOrZero<double>(a, [](auto x, auto& v) { return BrowserTranslatedMemory::Adapter().ReadFloat64(x, v); });
}
void FlatWrite8(std::uint32_t a, std::uint8_t v) {
    WriteOrFault(a, v, [](auto x, auto y) { return BrowserTranslatedMemory::Adapter().Write8(x, y); });
}
void FlatWrite16(std::uint32_t a, std::uint16_t v) {
    WriteOrFault(a, v, [](auto x, auto y) { return BrowserTranslatedMemory::Adapter().Write16(x, y); });
}
void FlatWrite32(std::uint32_t a, std::uint32_t v) {
    WriteOrFault(a, v, [](auto x, auto y) { return BrowserTranslatedMemory::Adapter().Write32(x, y); });
}
void FlatWrite64(std::uint32_t a, std::uint64_t v) {
    WriteOrFault(a, v, [](auto x, auto y) { return BrowserTranslatedMemory::Adapter().Write64(x, y); });
}
void FlatWriteFloat32(std::uint32_t a, double v) {
    WriteOrFault(a, v, [](auto x, auto y) { return BrowserTranslatedMemory::Adapter().WriteFloat32(x, static_cast<float>(y)); });
}
void FlatWriteFloat64(std::uint32_t a, double v) {
    WriteOrFault(a, v, [](auto x, auto y) { return BrowserTranslatedMemory::Adapter().WriteFloat64(x, y); });
}
void FlatWriteRam8(std::uint32_t a, std::uint8_t v) { FlatWrite8(a, v); }
void FlatWriteRam16(std::uint32_t a, std::uint16_t v) { FlatWrite16(a, v); }
void FlatWriteRam32(std::uint32_t a, std::uint32_t v) { FlatWrite32(a, v); }
void FlatWriteRam64(std::uint32_t a, std::uint64_t v) { FlatWrite64(a, v); }
void FlatWriteRamFloat32(std::uint32_t a, double v) { FlatWriteFloat32(a, v); }
void FlatWriteRamFloat64(std::uint32_t a, double v) { FlatWriteFloat64(a, v); }
} // namespace MemoryInline
