#include "own_backend.h"

#include <fd/console.h>
#include <fd/entity_cache.h>
#include <fd/hook.h>
#include <fd/library_info.h>
#include <fd/log.h>
#include <fd/netvar_storage.h>
#include <fd/render/context.h>
#include <fd/tool/string_view.h>
#include <fd/valve/client.h>
#include <fd/valve/engine.h>
#include <fd/valve/entity.h>
#include <fd/valve/entity_handle.h>
#include <fd/valve/entity_list.h>
#include <fd/valve/vgui.h>
#include <fd/valve_library_info.h>
#include <fd/vtable.h>

namespace fd
{
static bool context() noexcept;
}

static HINSTANCE self_handle;

#define HOOK_KNOWN_VFUNC(_NAME_, _OBJ_) BOOST_STRINGIZE(_NAME_), _OBJ_[&_NAME_]
#define RENDER_BACKEND                  IDirect3DDevice9

#ifdef FD_SHARED_LIB
static HANDLE thread;
static DWORD thread_id;

[[noreturn]]
static void exit_context(bool success)
{
    FreeLibraryAndExitThread(self_handle, success ? EXIT_SUCCESS : EXIT_FAILURE);
}

static bool lock_context()
{
    return SuspendThread(thread) != -1;
}

static bool unlock_context()
{
    return ResumeThread(thread) != -1;
}

[[noreturn]]
static DWORD WINAPI context_proxy(LPVOID ptr)
{
    self_handle  = static_cast<HINSTANCE>(ptr);
    auto success = fd::context();
    exit_context(success);
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

#else
#define USE_OWN_RENDER_BACKEND

int main(int argc, char *argv[])
{
    ignoer_unised(argc, argv);
    self_handle = GetModuleHandle(nullptr);
    auto result = fd::context();
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif

bool fd::context() noexcept
{
#ifdef _DEBUG
#ifdef FD_SHARED_LIB
    system_console console;
    if (!console.init())
        return false;
#endif
    logging_activator log_activator;
    if (!log_activator)
        return false;
#endif

    vtable<RENDER_BACKEND> render_vtable;
    HWND window;
    WNDPROC window_proc;

#ifdef USE_OWN_RENDER_BACKEND
    own_render_backend own_render(L"Unnamed", self_handle);
    if (!own_render.initialized())
        return false;

    render_vtable = own_render.device.get();
    window        = own_render.hwnd;
    window_proc   = own_render.info.lpfnWndProc;
#else
    auto shader_api_dll = system_library(L"shaderapidx9.dll");
    render_vtable       = [&] {
        auto val = shader_api_dll.pattern("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        return **reinterpret_cast<IDirect3DDevice9 ***>(static_cast<uint8_t *>(val) + 1);
    }();
    D3DDEVICE_CREATION_PARAMETERS creation_parameters;
    if (FAILED(render_vtable->GetCreationParameters(&creation_parameters)))
        return false;
    window      = creation_parameters.hFocusWindow;
    window_proc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(creation_parameters.hFocusWindow, GWLP_WNDPROC));

#endif

    render_context render;

    if (!render.init(window, render_vtable.instance()))
        return false;

#ifdef FD_SHARED_LIB
    valve_library client_dll(L"client.dll");
    valve_library engine_dll(L"engine.dll");
    valve_library vgui_dll(L"vguimatsurface.dll");

    valve::client v_client(client_dll.interface("VClient"));
    valve::engine v_engine(engine_dll.interface("VEngineClient"));
    valve::vgui_surface v_gui(vgui_dll.interface("VGUI_Surface"));
    valve::entity_list v_ent_list(client_dll.interface("VClientEntityList"));

    native_entity_finder entity_finder(bind(v_ent_list.get_client_entity, placeholders::_1));
    entity_cache cached_entity(&entity_finder);

    // todo: check if ingame and use exisiting player
    valve::entity csplayer_vtable(client_dll.vtable("C_CSPlayer"));

    netvar_storage netvars;
    netvars.store(v_client.get_all_classes());
    netvars.store(csplayer_vtable.get_desc_data_map());
    netvars.store(csplayer_vtable.get_prediction_data_map());
#endif

    hook_context hooks;

    hooks.create("WinAPI.WndProc", window_proc, [&](auto orig, auto... args) -> LRESULT {
        using result_t = render_context::process_result;
        result_t pmr;
        render.process_message(args..., &pmr);
        switch (pmr)
        {
        case result_t::idle:
            return orig(args...);
        case result_t::updated:
            return DefWindowProc(args...);
        case result_t::locked:
            return TRUE;
        default:
            std::unreachable();
        }
    });
#ifndef USE_OWN_RENDER_BACKEND
    hooks.create(HOOK_KNOWN_VFUNC(RENDER_BACKEND::Release, render_vtable), [&](auto &&orig) {
        auto refs = orig();
        if (refs == 0)
            render.detach();
        return refs;
    });
#endif
    hooks.create(HOOK_KNOWN_VFUNC(RENDER_BACKEND::Reset, render_vtable), [&](auto &&orig, auto... args) {
        render.reset();
        return orig(args...);
    });
    hooks.create(HOOK_KNOWN_VFUNC(RENDER_BACKEND::Present, render_vtable), [&](auto &&orig, auto... args) {
        if (auto frame = render.new_frame())
        {
            // #ifndef IMGUI_DISABLE_DEMO_WINDOWS
            ImGui::ShowDemoWindow();
            // #endif
        }
        return orig(args...);
    });
#ifdef FD_SHARED_LIB
    hooks.create("VGUI.ISurface::LockCursor", v_gui.lock_cursor, [&](auto &&orig) {
        // if (hack_menu.visible() && !this_ptr->IsCursorVisible() /*&& ifc.engine->IsInGame()*/)
        //{
        //     this_ptr->UnlockCursor();
        //     return;
        // }
        orig();
    });
    hooks.create("CHLClient::CreateMove", v_client[22].get<void, int, int, int>(), [&](auto &&orig, auto... args) {
        //
        orig(args...);
    });
    using entity_list_callback = void(__thiscall *)(void *, void *, valve::entity_handle);
    hooks.create(
        "CClientEntityList::OnAddEntity",
        void_to_func<entity_list_callback>(client_dll.pattern("55 8B EC 51 8B 45 0C 53 56 8B F1 57")),
        [&](auto &&orig, auto handle_interface, auto handle) {
            orig(handle_interface, handle);
            // todo: work with this_ptr
            cached_entity.add(handle.index());
        });
    hooks.create(
        "CClientEntityList::OnRemoveEntity",
        void_to_func<entity_list_callback>(client_dll.pattern("55 8B EC 51 8B 45 0C 53 8B D9 56 57 83 F8 FF 75 07")),
        [&](auto &&orig, auto handle_interface, auto handle) {
            // todo: work with this_ptr
            cached_entity.remove(handle.index());
            orig(handle_interface, handle);
        });
#endif

#if 1
    if (!hooks.enable_lazy())
        return false;
#else
    if (!hooks.enable())
        return false;
#endif

#ifdef USE_OWN_RENDER_BACKEND
    if (!own_render.run())
        return false;
#endif

#ifdef FD_SHARED_LIB
    if (!lock_context())
        return false;
#endif

#if 1
    if (!hooks.disable_lazy())
        return false;
#else
    if (!hooks.disable())
        return false;
#endif

    return true;
}