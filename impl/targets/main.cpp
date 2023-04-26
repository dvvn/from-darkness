#include "own_backend.h"

#include <fd/render/context.h>
#include <fd/render/context_init.h>
#include <fd/render/context_update.h>
#include <fd/render/frame.h>
// #include <fd/gui/context.h>
// #include <fd/gui/menu.h>
#include <fd/hooking/callback.h>
#include <fd/lazy_invoke.h>
#include <fd/logging/init.h>
#include <fd/vfunc.h>

#include <windows.h>

#include <algorithm>
#include <functional>

namespace fd
{
static bool enable_hooks(auto &rng)
{
    return std::all_of(std::begin(rng), std::end(rng), std::mem_fn(&basic_hook::enable));
}

static bool disable_hooks(auto &rng)
{
    return std::all_of(std::rbegin(rng), std::rend(rng), std::mem_fn(&basic_hook::disable));
}

static void destroy_hooks(auto &rng)
{
    std::for_each(std::rbegin(rng), std::rend(rng), std::destroy_at<basic_hook>);
}
} // namespace fd

// #define _WINDLL
#define FD_MODE_GUI_TEST

static bool context(HMODULE self_handle);

#if defined(FD_MODE_GUI_TEST) && !defined(_WINDLL)
#define USE_OWN_RENDER_BACKEND
#endif

#ifdef _WINDLL
static HANDLE thread;
static DWORD thread_id;

[[noreturn]]
static void exit_fail()
{
    FreeLibraryAndExitThread(nullptr /*WIP*/, EXIT_FAILURE);
}

static DWORD WINAPI context_proxy(LPVOID param)
{
    if (!context())
        exit_fail();
    return TRUE;
}

// ReSharper disable once CppInconsistentNaming
BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        thread = CreateThread(nullptr, 0, context_proxy, handle, 0, &thread_id);
        if (!thread)
            return EXIT_FAILURE;
        break;
    }
#if 0
    case DLL_THREAD_ATTACH: // Do thread-specific initialization.
        break;
    case DLL_THREAD_DETACH: // Do thread-specific cleanup.
        break;
#endif
    case DLL_PROCESS_DETACH:
        if (reserved != nullptr) // do not do cleanup if process termination scenario
        {
            break;
        }

        // Perform any necessary cleanup.
        break;
    }

    return EXIT_SUCCESS;
}
#else
int main(int argc, char *argv[])
{
    return context(GetModuleHandle(nullptr)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif

static bool context(HMODULE self_handle)
{
    using namespace fd;

    logger_registrar::start();
    [[maybe_unused]] //
    invoke_on_destruct stop_logging = logger_registrar::stop;

    render_context rctx;

#ifdef USE_OWN_RENDER_BACKEND
    auto backend = own_render_backend(L"Unnamed", self_handle);
    if (!backend.initialized())
        return false;
    if (!init(&rctx, backend.device, backend.hwnd))
        return false;
    auto target_wnd_proc = backend.info.lpfnWndProc;
#else
    //
    //

    if (!init(&rctx, ??, ??))
        return false;
#endif

    basic_hook *hooks[] = {
        make_hook_callback(
            "WinAPI.WndProc",
            target_wnd_proc,
            [&](auto orig, auto hwnd, auto... args) -> LRESULT {
                assert(rctx.window == hwnd);
                process_message_result pmr;
                process_message(&rctx, args..., &pmr);
                switch (pmr)
                {
                case process_message_result::idle:
                    return orig(hwnd, args...);
                case process_message_result::updated:
                    return DefWindowProc(hwnd, args...);
                case process_message_result::locked:
                    return TRUE;
                default:
                    std::unreachable();
                }
            }),
        make_hook_callback(
            "IDirect3DDevice9::Reset",
            from_void(vtable(rctx.backend).func(16), &IDirect3DDevice9::Reset),
            [&](auto orig, auto this_ptr, auto... args) {
                assert(rctx.backend == this_ptr);
                reset(&rctx);
                return orig(args...);
            }),
        make_hook_callback(
            "IDirect3DDevice9::Present",
            from_void(vtable(rctx.backend).func(17), &IDirect3DDevice9::Present),
            [&](auto orig, auto this_ptr, auto... args) {
                assert(rctx.backend == this_ptr);
                if (auto frame = render_frame(&rctx))
                {
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
                    ImGui::ShowDemoWindow();
#endif
                }
                return orig(args...);
            })};

    [[maybe_unused]] //
    invoke_on_destruct hooks_destroyer = [&] {
        destroy_hooks(hooks);
    };

    if (!enable_hooks(hooks))
        return false;

#ifdef USE_OWN_RENDER_BACKEND
    if (!backend.run())
        return false;
#else

#endif

#ifdef _WINDLL
    if (!disable_hooks(hooks))
        return false;
#endif

    return true;
}