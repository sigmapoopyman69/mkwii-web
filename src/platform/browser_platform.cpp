#include "browser_platform.h"

#include <emscripten.h>

namespace BrowserPlatform {

EM_JS(std::uint32_t, query_browser_capabilities, (), {
    const nav = globalThis.navigator || {};
    let flags = 0;
    if (nav.gpu) flags |= 1;
    if (typeof nav.getGamepads === "function") flags |= 2;
    if (typeof globalThis.AudioContext === "function" ||
        typeof globalThis.webkitAudioContext === "function") flags |= 4;
    if (typeof globalThis.indexedDB !== "undefined") flags |= 8;
    if (globalThis.crossOriginIsolated === true &&
        typeof globalThis.SharedArrayBuffer !== "undefined") flags |= 16;
    if (globalThis.document && document.fullscreenEnabled) flags |= 32;
    return flags;
});

EM_JS(double, browser_now_milliseconds, (), {
    return (globalThis.performance && typeof performance.now === "function")
        ? performance.now()
        : Date.now();
});

std::uint32_t GetCapabilities() noexcept {
    return query_browser_capabilities();
}

double NowMilliseconds() noexcept {
    return browser_now_milliseconds();
}

} // namespace BrowserPlatform
