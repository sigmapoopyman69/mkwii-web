#include <cstdio>
#include <emscripten/emscripten.h>

#include "platform/browser_platform.h"

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
    std::puts("WiiCompiled Web 0.4: browser platform layer initialized.");
    std::printf("Browser capability flags: %u\n", capabilities);
    std::puts("Game runtime and graphics backend are not connected yet.");
    return 0;
}
