#include "native_sources.h"
#include "debug/console.h"
#include "debug/log.h"
#include "hook/hook.h"
#include "hooked/wndproc.h"
#include "native/client.h"
#include "render/backend/native/dx9.h"
#include "render/backend/native/win32.h"
#include "render/context.h"

#include <Windows.h>

#include <cassert>
#include <cstdlib>
#include <exception>

static HINSTANCE self_handle;
static HANDLE thread;
static DWORD thread_id;

[[noreturn]]
static void exit_context(bool success)
{
    assert(GetCurrentThread() == thread);
    FreeLibraryAndExitThread(self_handle, success ? EXIT_SUCCESS : EXIT_FAILURE);
}

static bool lock_context()
{
    return SuspendThread(thread) != -1;
}

static bool resume_context()
{
    return ResumeThread(thread) != -1;
}

static void context();

static DWORD CALLBACK context_proxy(LPVOID ptr) noexcept
{
    try
    {
        self_handle = static_cast<HINSTANCE>(ptr);
        context();
        exit_context(true);
    }
    catch (std::exception const &ex)
    {
        exit_context(false);
    }
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
            return FALSE;
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

    return TRUE;
}

void context()
{
#ifdef _DEBUG
    fd::log_activator log_activator;
    fd::system_console console;
#endif

    fd::native_sources sources;
    fd::native_client client(sources.client);

    fd::render_context rctx;
    fd::win32_backend_native win32;
    fd::dx9_backend_native dx9(sources.shaderapidx9);

    fd::hook_context hooks;

    hooks.create("WinAPI.WndProc", win32.proc(), fd::hooked_wndproc(&win32));
}