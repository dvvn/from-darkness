#include "native_sources.h"
#include "object_holder.h"
#include "debug/console.h"
#include "debug/log.h"
#include "gui/menu.h"
#include "hook/preferred_backend.h"
#include "hooked/directx9.h"
#include "hooked/winapi.h"
#include "native/client.h"
#include "native/engine.h"
#include "native/entity_list.h"
#include "native/player.h"
#include "netvar/netvar_storage.h"
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

[[noreturn]]
static void exit_thread(bool const success)
{
    assert(GetCurrentThreadId() == thread_id);
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
BOOL WINAPI DllMain(HINSTANCE const handle, DWORD const reason, LPCVOID const reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        thread = CreateThread(
            nullptr, 0,
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
            handle, 0, &thread_id);
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

    fd::native_sources const sources;

    auto const menu           = fd::make_object<fd::menu>();
    auto const vars_sample    = fd::make_object<fd::vars_sample>();
    auto const netvars        = fd::make_object<fd::netvar_storage>();
    auto const render_context = fd::make_object<fd::render_context>();
    auto const system_backend = fd::make_object<fd::native_win32_backend>();
    auto const render_backend = fd::make_object<fd::native_dx9_backend>(sources.shaderapidx9);

    auto vars = join(vars_sample);

    fd::native_client const client(sources.client);
    fd::native_engine const engine(sources.engine);
    fd::native_entity_list const ents_list(sources.client);

    auto const player = engine.in_game() ? fd::native_player(ents_list, engine.local_player_index())
                                         : fd::native_player(sources.client);

    netvars->store(client.all_classes());
    netvars->store(player.data_map.description());
    netvars->store(player.data_map.prediction());

    auto const hook_backend = fd::make_object<fd::preferred_hook_backend>();

    auto const hk_wndproc = make_object<fd::hooked_wndproc>(system_backend);
    hook_backend->create(hk_wndproc);
    auto const hk_dx9_reset = make_object<fd::hooked_directx9_reset>(render_backend);
    hook_backend->create(hk_dx9_reset);
    auto const hk_dx9_present = make_object<fd::hooked_directx9_present>( //
        fd::render_frame(
            render_backend, system_backend,
            render_context, //
            menu, data(vars), size(vars)));
    hook_backend->create(hk_dx9_present);

#if 0 // rewrite
#ifndef FD_SPOOF_RETURN_ADDRESS
    fd::init_hook_callback<fd::hooked_verify_return_address>();
    hook_backend->create(fd::make_object<fd::hook_callback_ref<fd::hooked_verify_return_address>>( //
        sources.client.return_address_checker()));
#if 0
    hook_backend->create(fd::make_object<hooked_verify_return_address_ref>( //
        sources.shaderapidx9.return_address_checker()));
#endif
#endif
#endif

    hook_backend->enable();

    pause_thread();
}