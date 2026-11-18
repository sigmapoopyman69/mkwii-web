#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
namespace BrowserGuestMemory {
class Memory {
public:
    static constexpr std::uint32_t kPageSize = 4096;
    static constexpr std::uint32_t kMem1Size = 24u * 1024u * 1024u;
    static constexpr std::uint32_t kMem2Size = 128u * 1024u * 1024u;
    Memory();
    ~Memory();
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    void Reset() noexcept;
    bool Contains(std::uint32_t address, std::size_t length = 1) const noexcept;
    bool Read8(std::uint32_t address, std::uint8_t& value) const noexcept;
    bool Read16(std::uint32_t address, std::uint16_t& value) const noexcept;
    bool Read32(std::uint32_t address, std::uint32_t& value) const noexcept;
    bool ReadBytes(std::uint32_t address, void* destination, std::size_t length) const noexcept;
    bool Write8(std::uint32_t address, std::uint8_t value) noexcept;
    bool Write16(std::uint32_t address, std::uint16_t value) noexcept;
    bool Write32(std::uint32_t address, std::uint32_t value) noexcept;
    bool WriteBytes(std::uint32_t address, const void* source, std::size_t length) noexcept;
    std::size_t AllocatedPageCount() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
bool RunSmokeTest() noexcept;
}
