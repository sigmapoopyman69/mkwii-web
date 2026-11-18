#include <cstdio>
#include <emscripten/emscripten.h>

#include "platform/browser_platform.h"
#include "platform/browser_host_context.h"
#include "platform/browser_guest_memory.h"
#include "platform/browser_runtime_memory.h"
#include "platform/browser_translated_memory.h"

#ifdef WII_COMPILED_HAS_TRANSLATOR_SMOKE
extern "C" int mkwii_web_translator_smoke_test();
#endif

extern "C" {

EMSCRIPTEN_KEEPALIVE
int mkwii_web_status() { return 1; }

EMSCRIPTEN_KEEPALIVE
unsigned int mkwii_web_capabilities() { return BrowserPlatform::GetCapabilities(); }

EMSCRIPTEN_KEEPALIVE
double mkwii_web_uptime_ms() { return BrowserPlatform::NowMilliseconds(); }

}

int main() {
    const auto capabilities = BrowserPlatform::GetCapabilities();
    std::puts("WiiCompiled Web 0.9: upstream translator execution prototype initialized.");
    std::printf("Browser capability flags: %u\n", capabilities);
    const bool fiberSmokeTest = BrowserHostContext::RunSmokeTest();
    std::printf("Browser cooperative-fiber smoke test: %s\n", fiberSmokeTest ? "PASS" : "FAIL");
    const bool memorySmokeTest = BrowserGuestMemory::RunSmokeTest();
    std::printf("Browser guest-memory smoke test: %s\n", memorySmokeTest ? "PASS" : "FAIL");
    const bool runtimeMemorySmokeTest = BrowserRuntimeMemory::RunSmokeTest();
    std::printf("Browser runtime-memory API smoke test: %s\n", runtimeMemorySmokeTest ? "PASS" : "FAIL");
    const bool translatedMemorySmokeTest = BrowserTranslatedMemory::RunSmokeTest();
    std::printf("Browser translated-memory helper ABI smoke test: %s\n", translatedMemorySmokeTest ? "PASS" : "FAIL");
#ifdef WII_COMPILED_HAS_TRANSLATOR_SMOKE
    const bool translatorSmokeTest = mkwii_web_translator_smoke_test() == 1;
    std::printf("Upstream translator-generated PowerPC execution: %s (35 + 7 = 42)\n",
                translatorSmokeTest ? "PASS" : "FAIL");
#else
    const bool translatorSmokeTest = false;
    std::puts("Upstream translator-generated PowerPC execution: NOT BUILT");
#endif
    return (fiberSmokeTest && memorySmokeTest && runtimeMemorySmokeTest &&
            translatedMemorySmokeTest && translatorSmokeTest) ? 0 : 1;
}
