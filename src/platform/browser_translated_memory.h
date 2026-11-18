#pragma once

#include "browser_runtime_memory.h"

#include <cstdint>

namespace BrowserTranslatedMemory {

// Process-wide guest-memory instance used by emitted translated C++ helpers.
// This is a checked transitional ABI: it never returns host pointers.
BrowserGuestMemory::Memory& Backing() noexcept;
BrowserRuntimeMemory::Adapter& Adapter() noexcept;
void ClearFault() noexcept;
bool HasFault() noexcept;
bool RunSmokeTest() noexcept;

} // namespace BrowserTranslatedMemory

// Keep the helper spellings used by WiiCompiled's current flat-memory codegen.
// These checked implementations are intentionally slower than the upstream
// native flat-pointer path and reject unmapped/MMIO addresses.
namespace MemoryInline {
std::uint8_t FlatRead8(std::uint32_t address);
std::uint16_t FlatRead16(std::uint32_t address);
std::uint32_t FlatRead32(std::uint32_t address);
std::uint64_t FlatRead64(std::uint32_t address);
float FlatReadFloat32(std::uint32_t address);
double FlatReadFloat64(std::uint32_t address);

void FlatWrite8(std::uint32_t address, std::uint8_t value);
void FlatWrite16(std::uint32_t address, std::uint16_t value);
void FlatWrite32(std::uint32_t address, std::uint32_t value);
void FlatWrite64(std::uint32_t address, std::uint64_t value);
void FlatWriteFloat32(std::uint32_t address, double value);
void FlatWriteFloat64(std::uint32_t address, double value);

// Codegen uses this family only for stores proven to target ordinary RAM.
void FlatWriteRam8(std::uint32_t address, std::uint8_t value);
void FlatWriteRam16(std::uint32_t address, std::uint16_t value);
void FlatWriteRam32(std::uint32_t address, std::uint32_t value);
void FlatWriteRam64(std::uint32_t address, std::uint64_t value);
void FlatWriteRamFloat32(std::uint32_t address, double value);
void FlatWriteRamFloat64(std::uint32_t address, double value);
} // namespace MemoryInline
