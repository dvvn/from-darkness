#include "interface_holder.h"
#include "native_sources.h"
#include "debug/console.h"
#include "debug/log.h"
#include "gui/menu.h"
#include "hook/callback.h"
#include "hook/preferred_backend.h"
#include "hooked/directx9.h"
#include "hooked/winapi.h"
#include "native/client.h"
#include "render/backend/native/dx9.h"
#include "render/backend/native/win32.h"
#include "render/context.h"
#include "render/frame.h"
#include "vars/sample.h"

#include <Windows.h>
#include <d3d9.h>

#include <cassert>

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

// ReSharper disable once CppInconsistentNaming
BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        thread = CreateThread(
            nullptr,
            0,
            [](LPVOID ptr) -> DWORD {
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
            },
            handle,
            0,
            &thread_id
        );
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

    using enum fd::interface_type;

    auto menu           = fd::make_interface<fd::menu, stack>();
    auto vars_sample    = fd::make_interface<fd::vars_sample, stack>();
    auto render_context = fd::make_interface<fd::render_context, in_place>();
    auto system_backend = fd::make_interface<fd::native_win32_backend, in_place>();
    auto render_backend = fd::make_interface<fd::native_dx9_backend, in_place>(sources.shaderapidx9);

    auto vars = join(vars_sample);
    fd::render_frame_full render_frame( //
        FD_GROUP_ARGS(render_backend, system_backend, render_context),
        FD_GROUP_ARGS(menu, data(vars), size(vars))
    );

    auto hook_backend = fd::make_interface<fd::preferred_hook_backend, in_place>();
    fd::vtable render_vtable(render_backend->get());
    hook_backend->create(prepare_hook<fd::hooked_wndproc>( //
        system_backend->proc(),
        system_backend
    ));
    hook_backend->create(prepare_hook<fd::hooked_dx9_reset>( //
        render_vtable[&IDirect3DDevice9::Reset],
        render_backend
    ));
    hook_backend->create(prepare_hook<fd::hooked_dx9_present>( //
        render_vtable[&IDirect3DDevice9::Present],
        &render_frame
    ));

    hook_backend->enable();

    pause_thread();
}