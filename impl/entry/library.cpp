#include "interface_holder.h"
#include "native_sources.h"
#include "debug/console.h"
#include "debug/log.h"
#include "hook/callback.h"
#include "hook/preferred_backend.h"
#include "hooked/directx9.h"
#include "hooked/winapi.h"
#include "native/client.h"
#include "render/backend/native/dx9.h"
#include "render/backend/native/win32.h"
#include "render/context.h"

#include <Windows.h>
#include <d3d9.h>

#include <cassert>
#include <cstdlib>
#include <exception>

static HINSTANCE self_handle;
static HANDLE thread;
static DWORD thread_id;

static void DECLSPEC_NORETURN exit_thread(bool success)
{
    assert(GetCurrentThread() == thread);
    FreeLibraryAndExitThread(self_handle, success ? EXIT_SUCCESS : EXIT_FAILURE);
}

static bool pause_thread()
{
    return SuspendThread(thread) != -1;
}

static bool resume_thread()
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
        exit_thread(true);
    }
    catch (std::exception const &ex)
    {
        exit_thread(false);
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
    fd::system_console console;
    fd::log_activator log_activator;
#endif

    fd::native_sources sources;
    fd::native_client client(sources.client);

    auto render = fd::make_interface<fd::render_context>();
    fd::win32_backend_native win32;
    fd::dx9_backend_native dx9(sources.shaderapidx9);

    auto hook_backend = fd::make_interface<fd::preferred_hook_backend>();

    hook_backend->create(prepare_hook<fd::hooked_wndproc>(win32.proc(), &win32));
    hook_backend->create(prepare_hook<fd::hooked_dx9_reset>(dx9[&IDirect3DDevice9::Reset], &dx9));
    hook_backend->create(prepare_hook<fd::hooked_dx9_present>(dx9[&IDirect3DDevice9::Present], &dx9, &win32, &render));

    hook_backend->enable();

    pause_thread();
}