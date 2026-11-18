#include "browser_guest_abi_smoke.h"

extern "C" void wii_compiled_translated_branch_smoke(CpuContext* ctx);
extern "C" void wii_compiled_translated_memory_smoke(CpuContext* ctx);

extern "C" int mkwii_web_translator_smoke_test() {
    CpuContext context{};

    // Exercise both sides of the translated conditional branch.
    context.gpr[3] = 42;
    wii_compiled_translated_branch_smoke(&context);
    if (context.gpr[4] != 7) return 1;

    context = {};
    context.gpr[3] = 41;
    wii_compiled_translated_branch_smoke(&context);
    if (context.gpr[4] != 9) return 2;

    // Execute translated lwz/addi/stw against the browser guest-memory adapter.
    BrowserTranslatedMemory::Backing().Reset();
    BrowserTranslatedMemory::ClearFault();
    if (!BrowserTranslatedMemory::Backing().Write32(0x80002000u, 0x12345678u)) return 3;

    context = {};
    context.gpr[3] = 0x80002000u;
    wii_compiled_translated_memory_smoke(&context);
    if (BrowserTranslatedMemory::HasFault()) return 4;
    if (context.gpr[4] != 0x12345678u) return 5;

    std::uint32_t stored = 0;
    if (!BrowserTranslatedMemory::Backing().Read32(0x80002004u, stored)) return 6;
    if (stored != 0x12345679u) return 7;
    return 0;
}
