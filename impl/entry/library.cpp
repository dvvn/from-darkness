#include "native_sources.h"
#include "object_holder.h"
#include "debug/console.h"
#include "debug/log.h"
#include "functional/function_holder.h"
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

static void context();

#if 1
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
    return SuspendThread(thread) != static_cast<DWORD>(-1);
}

static bool resume_thread()
{
    return ResumeThread(thread) != static_cast<DWORD>(-1);
}

// ReSharper disable once CppInconsistentNaming
BOOL WINAPI DllMain(HINSTANCE handle, DWORD const reason, LPCVOID const reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        self_handle = handle;
        thread      = CreateThread(
            nullptr, 0,
            [](auto) -> DWORD {
                try
                {
                    context();
                    exit_thread(true);
                }
                catch (std::exception const &ex)
                {
                    exit_thread(false);
                }
            },
            nullptr, 0, &thread_id);
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
#endif

static auto get_player_data_map(
    fd::native_sources const &sources, //
    fd::native_engine engine, fd::native_entity_list ents)
{
    fd::native_player player;
    if (engine.in_game())
        player = {ents.get_client_entity(engine.local_player_index())};
    else
        player = {sources.client};

    return player.data_map;
}

void context()
{
#ifdef _DEBUG
    fd::system_console console;
    fd::log_activator log_activator;
#endif

    fd::native_sources const sources;

    using fd::make_object;

    auto const netvars        = make_object<fd::netvar_storage>();
    auto const render_context = make_object<fd::render_context>();
    auto const system_backend = make_object<fd::native_win32_backend>();
    auto const render_backend = make_object<fd::native_dx9_backend>(sources.shaderapidx9);

    // todo: add macro to disable unload (and drop mutex usage)
    constexpr fd::function_holder unload_handler(resume_thread, std::in_place_type<void>);

    auto const menu = make_object<fd::menu>(&unload_handler);

    fd::native_client const client(sources.client);
    fd::native_engine const engine(sources.engine);
    fd::native_entity_list const ents_list(sources.client);

    auto const player_data_map = get_player_data_map(sources, engine, ents_list);

    netvars->store(client.all_classes());
    netvars->store(player_data_map.description());
    netvars->store(player_data_map.prediction());

    fd::render_frame const render_frame(
        {render_backend, system_backend, render_context, menu}, //
        {nullptr, 0});

    auto const hk_wndproc     = make_object<fd::hooked_wndproc>(system_backend);
    auto const hk_dx9_reset   = make_object<fd::hooked_directx9_reset>(render_backend);
    auto const hk_dx9_present = make_object<fd::hooked_directx9_present>(&render_frame);

    auto const hook_backend = fd::make_object<fd::preferred_hook_backend>();
    hook_backend->create(hk_wndproc);
    hook_backend->create(hk_dx9_reset);
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