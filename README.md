# WiiCompiled Web

An experimental browser-port project for [WiiCompiled](https://github.com/patchzyy/Wiicompiled).

**Current status: WebAssembly build-system bootstrap only. This is not a playable Mario Kart Wii port.**

The goal is to investigate adapting WiiCompiled's static-recompilation output and runtime for browsers, with a single-file HTML launcher and browser-delivered WebAssembly resources.

## Chromebook-friendly workflow

No local Linux environment is required.

1. Edit source files in the GitHub web interface.
2. GitHub Actions builds the C++ WebAssembly module on a hosted Ubuntu runner.
3. The workflow commits generated `dist/mkwii-web.js` and `dist/mkwii-web.wasm` files to this repository.
4. `index.html` loads those files from this repository through jsDelivr.

After the first successful Actions run, open the GitHub Pages site (once Pages is enabled) or serve `index.html` from another static host. The launcher is intentionally a single HTML file; generated engine resources remain separate files in `dist/`.

## Build manually (optional)

With Emscripten and CMake installed:

```sh
emcmake cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The output is written to `dist/`.

## Current files

- `index.html` — single-file launcher and status UI.
- `src/main.cpp` — tiny C++ smoke-test module.
- `CMakeLists.txt` — Emscripten build configuration.
- `.github/workflows/build-wasm.yml` — remote build and resource publishing.

## Roadmap

1. Establish a repeatable Emscripten build (current milestone).
2. Audit WiiCompiled runtime dependencies and separate platform-independent game/runtime code from native OS integrations.
3. Investigate browser-compatible SDL, graphics/WebGPU, audio, input, storage, and file-loading paths.
4. Port the minimum runtime layer needed to execute translated game code.
5. Add a local user-provided game-data workflow only after a technically sound and legally appropriate design exists.

## Important technical limitation

The upstream WiiCompiled project currently targets native desktop platforms. Its runtime and Aurora graphics integration are not automatically browser-compatible just because Emscripten can compile C++. A substantial port is required. The current build only verifies that this repository can produce and load a small WebAssembly module.

The translator also generates game-specific C++ from a user's own game dump. This repository does not include Nintendo game data, game assets, translated game code, or a prebuilt game. Do not upload copyrighted game dumps or generated game content to this repository or public build services.

## Upstream licensing

WiiCompiled is licensed under GPL-3.0. Any code incorporated from WiiCompiled must retain its applicable notices and comply with the upstream license. See the upstream [LICENSE](https://github.com/patchzyy/Wiicompiled/blob/main/LICENSE).
