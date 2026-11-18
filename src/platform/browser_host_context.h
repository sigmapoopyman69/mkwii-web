#pragma once

#include <cstddef>

namespace BrowserHostContext {

using Handle = void*;
using Entry = void (*)(void*);

// Emscripten's cooperative-fiber backend. All contexts must remain on the
// browser's main thread; this is not a pthread or a parallel worker API.
bool InitializeScheduler(Handle* scheduler);
void ShutdownScheduler(Handle scheduler);
Handle Create(std::size_t stackSize, Entry entry, void* argument);
void Destroy(Handle context);
bool IsCurrent(Handle context);
void Switch(Handle target);

// Exercises two round-trips between a browser fiber and the main context.
bool RunSmokeTest();

} // namespace BrowserHostContext
