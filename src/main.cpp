#include <cstdio>
#include <emscripten/emscripten.h>

#include "platform/browser_platform.h"
#include "platform/browser_host_context.h"

extern "C" {

EMSCRIPTEN_KEEPALIVE
int mkwii_web_status() {
    return 1;
}

EMSCRIPTEN_KEEPALIVE
unsigned int mkwii_web_capabilities() {
    return BrowserPlatform::GetCapabilities();
}

EMSCRIPTEN_KEEPALIVE
double mkwii_web_uptime_ms() {
    return BrowserPlatform::NowMilliseconds();
}

}

int main() {
    const auto capabilities = BrowserPlatform::GetCapabilities();
    std::puts("WiiCompiled Web 0.5: browser runtime port prototype initialized.");
    std::printf("Browser capability flags: %u\n", capabilities);
    std::puts("Game runtime and graphics backend are not connected yet.");
    const bool fiberSmokeTest = BrowserHostContext::RunSmokeTest();
    std::printf("Browser cooperative-fiber smoke test: %s\n", fiberSmokeTest ? "PASS" : "FAIL");
    return 0;
}
