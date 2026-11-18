#include "guest_abi/guest_abi.h"

extern "C" void wii_compiled_translated_addi_smoke(CpuContext* ctx);

extern "C" int mkwii_web_translator_smoke_test() {
    CpuContext context{};
    context.gpr[3] = 35;
    wii_compiled_translated_addi_smoke(&context);
    return context.gpr[3] == 42 ? 1 : 0;
}
