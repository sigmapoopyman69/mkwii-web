#include <cstdio>
#include <emscripten/emscripten.h>

extern "C" {
EMSCRIPTEN_KEEPALIVE
int mkwii_web_status() {
    return 1;
}
}

int main() {
    std::puts("WiiCompiled Web: Emscripten bootstrap initialized.");
    std::puts("This is a build-system smoke test, not the game runtime.");
    return 0;
}
