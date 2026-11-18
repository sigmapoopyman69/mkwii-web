# WiiCompiled Web

An experimental browser-port project for [WiiCompiled](https://github.com/patchzyy/Wiicompiled).

**Current status: version 0.3 browser input/audio adapters. This is not a playable Mario Kart Wii port.**

The goal is to investigate adapting WiiCompiled's static-recompilation output and runtime for browsers, with a single HTML launcher and repository-hosted WebAssembly resources.

## Version 0.3

This milestone replaces the original hello-world-only C++ source with the first browser host-services boundary:

- A C++ browser platform module, isolated under `src/platform/`.
- Runtime capability detection for WebGPU, Gamepad API, Web Audio, IndexedDB, shared memory isolation, and fullscreen.
- A monotonic browser clock exposed to C++.
- Exported WebAssembly functions so the launcher can display the detected capabilities.
- GitHub Actions compiles the module and commits the generated `dist/` resources to this repository.

The launcher retains the real WebGPU adapter/device, canvas surface, WGSL shader pipeline, and animated frame loop from v0.2. It now also:
- Polls the browser Gamepad API and displays the connected controller, pressed button indices, and two stick pairs.
- Handles gamepad connect/disconnect events.
- Initializes Web Audio only after the user presses the audio test button, then plays a short 440 Hz test tone.
- Reports unavailable APIs and audio initialization errors in the page.

These are browser adapter tests, not Mario Kart controls or game audio. No translated Wii game code is executed.

## Chromebook-friendly workflow

No local Linux environment is required.

1. Edit source files in the GitHub web interface.
2. GitHub Actions builds the C++ WebAssembly module on a hosted Ubuntu runner.
3. The workflow commits generated `dist/mkwii-web.js` and `dist/mkwii-web.wasm` files to this repository.
4. `index.html` loads the files from this repository's own GitHub Pages origin.
5. `sw.js` caches the launcher and generated resources for repeat visits when served over HTTPS.

Enable GitHub Pages in repository Settings → Pages → Deploy from a branch → `main` → `/(root)`.

## Build manually (optional)

With Emscripten and CMake installed:

```sh
emcmake cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The output is written to `dist/`.

## Project structure

- `index.html` — single-file launcher and browser diagnostics.
- `src/main.cpp` — WebAssembly entry point and exported C API.
- `src/platform/browser_platform.h/.cpp` — browser host-services boundary.
- `CMakeLists.txt` — Emscripten build configuration.
- `.github/workflows/build-wasm.yml` — remote build and resource publishing.
- `sw.js` — same-origin offline resource cache.

## Roadmap

1. **0.1 — Browser platform foundation:** API detection, monotonic clock, build pipeline.
2. **0.2 — Browser graphics prototype:** establish a real WebGPU device, surface, and clear-frame loop.
3. **0.3 — Input/audio adapters:** browser gamepad mapping and Web Audio output.
4. **0.4 — Runtime dependency port:** identify and adapt the minimum portable WiiCompiled runtime components.
5. **0.5+ — Translated-code integration:** build and run compatible generated code after the runtime and graphics layers are ready.

## Technical limitation

Upstream WiiCompiled currently has a native desktop runtime and a separate Aurora graphics dependency. Its runtime CMake configuration explicitly targets native desktop platforms and 64-bit Clang; Emscripten support is not a switch that can simply be enabled. A substantial platform and graphics port is required.

The translator is a build-time tool. This repository does not include Nintendo game data, game assets, translated game code, or a prebuilt game. Keep any game-data workflow local to the user and do not upload copyrighted game dumps or generated game content to this repository or public build services.

## Upstream licensing

WiiCompiled is licensed under GPL-3.0. Any code incorporated from WiiCompiled must retain its applicable notices and comply with the upstream license. See the upstream [LICENSE](https://github.com/patchzyy/Wiicompiled/blob/main/LICENSE).
