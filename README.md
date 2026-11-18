# WiiCompiled Web

An experimental browser-port project for [WiiCompiled](https://github.com/patchzyy/Wiicompiled).

**Current status: version 0.10 multi-instruction translation prototype. The browser executes upstream-translated conditional branches and guest-memory loads/stores from synthetic PowerPC functions. This is not a playable Mario Kart Wii port.**

The goal is to investigate adapting WiiCompiled's static-recompilation output and runtime for browsers, with a single HTML launcher and repository-hosted WebAssembly resources.

## Version 0.4

This milestone adds a browser-local persistence layer:

- IndexedDB database for settings and user-selected files.
- Storage usage/quota estimate when supported by the browser.
- File listing and deletion controls.
- A small settings save/load round-trip test.
- An experimental Emscripten virtual-filesystem mount for stored files up to 128 MiB.
- Files stay in the current browser's local origin storage; the site does not upload them.

Large disc images are not yet suitable for this in-memory mount. A streaming filesystem and a game-runtime file-access contract are still required before large game data can be consumed.

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

## Version 0.5 — Runtime porting prototype

The first runtime primitive being adapted is cooperative host-context switching. The browser build now includes an Emscripten-fiber implementation with separate C/Asyncify stacks and a startup smoke test that switches into a fiber and back twice.

This does not yet run upstream WiiCompiled scheduler code. The upstream runtime still depends on native guest-memory mapping, Aurora, SDL, and desktop host services. See [the v0.5 porting notes](docs/v0.5-runtime-porting.md) for the dependency audit and next gates.

## Roadmap

1. **0.1 — Browser platform foundation:** API detection, monotonic clock, build pipeline.
2. **0.2 — Browser graphics prototype:** establish a real WebGPU device, surface, and clear-frame loop.
3. **0.3 — Input/audio adapters:** browser gamepad mapping and Web Audio output.
4. **0.4 — Browser storage adapters:** IndexedDB persistence and bounded virtual-filesystem mounting.
5. **0.5 — Runtime porting:** establish the experimental cooperative-fiber host-context boundary.
6. **0.6 — Guest memory:** sparse MEM1/MEM2 address model and alias tests.
7. **0.7 — Runtime memory API:** checked scalar, floating-point, and bulk-copy adapter.
8. **0.8 — Translated-memory ABI:** expose checked browser implementations of the upstream FlatRead/FlatWrite helper names.
9. **0.9 — First translated code:** use the upstream translator to emit a synthetic PowerPC function, compile it into the browser WASM module, and execute it.
10. **0.10 — Multi-instruction programs:** translate and execute conditional branches plus guest-memory loads/stores in WebAssembly.
11. **0.11+ — Broader runtime integration:** expand instruction coverage and connect more upstream runtime services.

## Technical limitation

Upstream WiiCompiled currently has a native desktop runtime and a separate Aurora graphics dependency. Its runtime CMake configuration explicitly targets native desktop platforms and 64-bit Clang; Emscripten support is not a switch that can simply be enabled. A substantial platform and graphics port is required.

The translator is a build-time tool. This repository does not include Nintendo game data, game assets, translated game code, or a prebuilt game. Keep any game-data workflow local to the user and do not upload copyrighted game dumps or generated game content to this repository or public build services.

## Version 0.8 — Translated-memory helper ABI

Adds a process-wide checked guest-memory instance and browser implementations of the upstream `MemoryInline::FlatRead*`, `FlatWrite*`, and `FlatWriteRam*` helper names. This is the first interface shaped for generated C++ to call. A hand-written generated-code-style smoke block validates reads, writes, aliases, and fault latching. It is not yet output from the upstream translator; direct pointers, the native 4 GiB mapping, MMIO/HLE, deferred reads, executable-write guards, and hardware-specific PowerPC float narrowing remain unported. See [the v0.8 ABI notes](docs/v0.8-translated-memory-abi.md).

## Version 0.9 — First upstream-translated function

The hosted build checks out WiiCompiled at pinned revision `03151e0a3429adb92dd9082fb815fbf194df3c1d`, builds its .NET translator core, and translates a synthetic PowerPC function containing `addi r3,r3,7` followed by `blr`. Emscripten compiles the generated C++ into the same browser WebAssembly module. At startup, a CPU-context wrapper calls the generated function with r3=35 and verifies r3=42. No game binary or game assets are used. See [the v0.9 translator smoke notes](docs/v0.9-translator-smoke.md).

## Version 0.10 — Branches and guest-memory access

The build now translates two synthetic PowerPC functions with the pinned upstream translator. One exercises `cmpwi` and `beq` with both taken and not-taken paths. The other executes `lwz`, `addi`, and `stw` against the browser guest-memory adapter. The browser checks the resulting register and stored memory values. See [the v0.10 translator program notes](docs/v0.10-translator-programs.md).

## Upstream licensing

WiiCompiled is licensed under GPL-3.0. Any code incorporated from WiiCompiled must retain its applicable notices and comply with the upstream license. See the upstream [LICENSE](https://github.com/patchzyy/Wiicompiled/blob/main/LICENSE).


## Version 0.6 — Guest-memory port prototype

Adds an independently written, sparse WebAssembly-compatible model of Wii MEM1 (24 MiB) and MEM2 (128 MiB). Physical, cached, and uncached address aliases share the same backing pages. Memory pages allocate on first write, scalar accessors use PowerPC big-endian byte order, and invalid accesses are rejected. A startup smoke test checks aliasing, bounds, sparse allocation, and reset.

This is not yet upstream's Memory/MemoryInline implementation and does not implement MMIO, deferred framebuffer reads, executable write guards, or direct translated-code pointers. See [the v0.6 guest-memory notes](docs/v0.6-guest-memory.md).


## Version 0.7 — Runtime memory API adapter

Adds a checked browser-side adapter for the portable subset of WiiCompiled's guest-memory contract: 8/16/32/64-bit scalar reads and writes, 32/64-bit floating-point values using PowerPC big-endian byte order, range checks, and bulk copies between guest memory and host buffers. The sparse backing store now supports multi-page byte ranges without requiring a contiguous host pointer.

The adapter is independently implemented and tested against the upstream API shape; it does not import the upstream runtime. Raw guest pointers and the translated-code 4 GiB flat-memory fast path are deliberately not exposed. Upstream uses fixed virtual-address reservations, page protection, MMIO/deferred-read traps, and executable-write guards; those mechanisms need a separate WebAssembly design before translated code can be integrated. See [the v0.7 runtime-memory notes](docs/v0.7-runtime-memory.md).
