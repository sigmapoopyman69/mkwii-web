#include "browser_host_context.h"

#include <emscripten/fiber.h>

#include <cstdint>
#include <cstdlib>
#include <new>

namespace BrowserHostContext {
namespace {

constexpr std::size_t kAsyncifyStackSize = 64 * 1024;

struct Context {
    emscripten_fiber_t fiber{};
    void* cStack = nullptr;
    void* asyncifyStack = nullptr;
    std::size_t cStackSize = 0;
    Entry entry = nullptr;
    void* argument = nullptr;
};

thread_local Context* g_current = nullptr;

void FiberEntry(void* opaque) {
    auto* context = static_cast<Context*>(opaque);
    g_current = context;
    context->entry(context->argument);

    // A guest fiber must explicitly switch back to its caller. Returning from
    // the Emscripten fiber entry would terminate the whole WebAssembly program.
    std::abort();
}

void Release(Context* context) {
    if (!context) return;
    std::free(context->cStack);
    std::free(context->asyncifyStack);
    delete context;
}

struct SmokeState {
    Handle scheduler = nullptr;
    unsigned step = 0;
};

void SmokeFiber(void* opaque) {
    auto* state = static_cast<SmokeState*>(opaque);
    state->step = 1;
    Switch(state->scheduler);
    state->step = 2;
    Switch(state->scheduler);
    std::abort();
}

} // namespace

bool InitializeScheduler(Handle* scheduler) {
    if (!scheduler || g_current) return false;

    auto* context = new (std::nothrow) Context();
    if (!context) return false;

    context->asyncifyStack = std::malloc(kAsyncifyStackSize);
    if (!context->asyncifyStack) {
        Release(context);
        return false;
    }

    emscripten_fiber_init_from_current_context(
        &context->fiber, context->asyncifyStack, kAsyncifyStackSize);

    g_current = context;
    *scheduler = context;
    return true;
}

void ShutdownScheduler(Handle scheduler) {
    auto* context = static_cast<Context*>(scheduler);
    if (!context || context != g_current) return;
    g_current = nullptr;
    Release(context);
}

Handle Create(std::size_t stackSize, Entry entry, void* argument) {
    if (!g_current || !entry || stackSize < 16 * 1024) return nullptr;

    auto* context = new (std::nothrow) Context();
    if (!context) return nullptr;

    context->cStackSize = (stackSize + 15u) & ~std::size_t(15u);
    // malloc provides max_align_t alignment, which is sufficient for Emscripten fibers.
    context->cStack = std::malloc(context->cStackSize);
    context->asyncifyStack = std::malloc(kAsyncifyStackSize);
    context->entry = entry;
    context->argument = argument;

    if (!context->cStack || !context->asyncifyStack) {
        Release(context);
        return nullptr;
    }

    emscripten_fiber_init(
        &context->fiber,
        FiberEntry,
        context,
        context->cStack,
        context->cStackSize,
        context->asyncifyStack,
        kAsyncifyStackSize);
    return context;
}

void Destroy(Handle handle) {
    auto* context = static_cast<Context*>(handle);
    if (!context || context == g_current) return;
    Release(context);
}

bool IsCurrent(Handle handle) {
    return handle != nullptr && handle == g_current;
}

void Switch(Handle target) {
    auto* destination = static_cast<Context*>(target);
    Context* source = g_current;
    if (!source || !destination || source == destination) return;

    g_current = destination;
    emscripten_fiber_swap(&source->fiber, &destination->fiber);
    g_current = source;
}

bool RunSmokeTest() {
    Handle scheduler = nullptr;
    if (!InitializeScheduler(&scheduler)) return false;

    SmokeState state;
    state.scheduler = scheduler;
    Handle fiber = Create(64 * 1024, SmokeFiber, &state);
    if (!fiber) {
        ShutdownScheduler(scheduler);
        return false;
    }

    Switch(fiber);
    const bool firstSwitch = state.step == 1;
    if (firstSwitch) Switch(fiber);
    const bool secondSwitch = state.step == 2;

    Destroy(fiber);
    ShutdownScheduler(scheduler);
    return firstSwitch && secondSwitch;
}

} // namespace BrowserHostContext
