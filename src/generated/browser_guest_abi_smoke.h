#pragma once

// Minimal CPU-context ABI mirror for the arithmetic-only translator smoke test.
// Field order and scalar widths match upstream runtime/include/guest_abi/cpu_context.h.
// This deliberately avoids importing desktop-only SIMD and FP-control headers.
#include <cstdint>

#ifndef MKW_RESTRICT
#define MKW_RESTRICT __restrict
#endif

union PPC_FPR {
    std::uint64_t raw;
    double d;
    struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        float ps1;
        float ps0;
#else
        float ps0;
        float ps1;
#endif
    } paired;
};

struct CpuContext {
    std::uint32_t gpr[32];
    std::uint32_t cr;
    std::uint32_t lr;
    std::uint32_t ctr;
    std::uint32_t xer;
    std::uint32_t fpscr;
    std::uint32_t pc;
    PPC_FPR fpr[32];
    std::uint32_t gqr[8];
    std::uint32_t hid0;
    std::uint32_t hid1;
    std::uint32_t hid2;
    std::uint32_t srr0;
    std::uint32_t srr1;
    std::uint32_t msr;
};
