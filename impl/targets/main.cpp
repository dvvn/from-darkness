#include "own_backend.h"

#include <fd/console.h>
#include <fd/hook.h>
#include <fd/lazy_invoke.h>
#include <fd/library_info.h>
#include <fd/log.h>
#include <fd/netvars/core.h>
#include <fd/players/cache.h>
#include <fd/render/context.h>
#include <fd/tool/string_view.h>
#include <fd/valve/entity_handle.h>
#include <fd/valve_interface.h>
#include <fd/valve_library_info.h>
#include <fd/vtable.h>

#include <windows.h>
#include <d3d9.h>

#include <algorithm>

static bool context(HINSTANCE self_handle) noexcept;

#ifdef FD_SHARED_LIB
static HANDLE thread;
static DWORD thread_id;

static DWORD WINAPI context_proxy(LPVOID ptr)
{
    auto result = context(static_cast<HINSTANCE>(ptr));
    FreeLibraryAndExitThread(static_cast<HMODULE>(ptr), result ? EXIT_SUCCESS : EXIT_FAILURE);
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

namespace fd::valve
{
extern vtable<void> entity_list_interface;
} // namespace fd::valve

#else
#define USE_OWN_RENDER_BACKEND

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    bool result;

    try
    {
        result = context(GetModuleHandle(nullptr));
    }
    catch (...)
    {
        result = false;
    }

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif

#define SECOND_ARG(_A_, _B_, ...) _B_
#define DUMMY_VAR                 BOOST_JOIN(dummy, __COUNTER__)

#define MAKE_DESTRUCTOR(_FN_, ...) \
    [[maybe_unused]]               \
    invoke_on_destruct SECOND_ARG(unused, ##__VA_ARGS__, const DUMMY_VAR) = _FN_;

bool context(HINSTANCE self_handle) noexcept
{
    (void)self_handle;

    using namespace fd;

#ifdef _DEBUG
#ifdef FD_SHARED_LIB
    if (!create_system_console())
        return false;
    MAKE_DESTRUCTOR(destroy_system_console);
#endif
    if (!init_logging())
        return false;
#endif

    log("Started!");
    MAKE_DESTRUCTOR([] { log("Finished!"); });

    vtable<IDirect3DDevice9> render_vtable;
    HWND window;
    WNDPROC window_proc;

#ifdef USE_OWN_RENDER_BACKEND
    auto own_render = own_render_backend(L"Unnamed", self_handle);
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

    render_context render_ctx;

    if (!render_ctx.init(window, render_vtable.instance()))
        return false;

#ifdef FD_SHARED_LIB
    auto client_dll = valve_library(L"client.dll");
    auto engine_dll = valve_library(L"engine.dll");
    auto vgui_dll   = valve_library(L"vguimatsurface.dll");

    vtable client_interface      = client_dll.interface("VClient");
    // vtable engine_interface    = engine_dll.interface("VEngineClient");
    vtable vgui_interface        = vgui_dll.interface("VGUI_Surface");
    valve::entity_list_interface = client_dll.interface("VClientEntityList");

    // todo: check if ingame and use exisiting player
    auto player_vtable = client_dll.vtable("C_CSPlayer");

    store_netvars(client_interface);
    store_extra_netvars(player_vtable);
    store_custom_netvars(client_dll);
#endif

    hook_context hooks;

    hooks.create("WinAPI.WndProc", window_proc, [&](auto orig, auto... args) -> LRESULT {
        using result_t = render_context::process_result;
        result_t pmr;
        render_ctx.process_message(args..., &pmr);
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
    hooks.create("IDirect3DDevice9::Release", render_vtable[&IDirect3DDevice9::Release], [&](auto &&orig) {
        auto refs = orig();
        if (refs == 0)
            render_ctx.detach();
        return refs;
    });
#endif
    hooks.create("IDirect3DDevice9::Reset", render_vtable[&IDirect3DDevice9::Reset], [&](auto &&orig, auto... args) {
        render_ctx.reset();
        return orig(args...);
    });
    hooks.create(
        "IDirect3DDevice9::Present", render_vtable[&IDirect3DDevice9::Present], [&](auto &&orig, auto... args) {
            if (auto frame = render_ctx.render_frame())
            {
                // #ifndef IMGUI_DISABLE_DEMO_WINDOWS
                ImGui::ShowDemoWindow();
                // #endif
            }
            return orig(args...);
        });
#ifdef FD_SHARED_LIB
    hooks.create("VGUI.ISurface::LockCursor", vgui_interface[67].get<void>(), [&](auto &&orig) {
        // if (hack_menu.visible() && !this_ptr->IsCursorVisible() /*&& ifc.engine->IsInGame()*/)
        //{
        //     this_ptr->UnlockCursor();
        //     return;
        // }
        orig();
    });
    hooks.create(
        "CHLClient::CreateMove", client_interface[22].get<void, int, int, int>(), [&](auto &&orig, auto... args) {
            //
            orig(args...);
        });
    using entity_list_callback = void(__thiscall *)(void *, void *, valve::entity_handle);
    hooks.create(
        "CClientEntityList::OnAddEntity",
        reinterpret_cast<entity_list_callback>(client_dll.pattern("55 8B EC 51 8B 45 0C 53 56 8B F1 57")),
        [&](auto &&orig, auto handle_interface, auto handle) {
            orig(handle_interface, handle);
            // todo: work with this_ptr
            on_add_entity(handle.index());
        });
    hooks.create(
        "CClientEntityList::OnRemoveEntity",
        reinterpret_cast<entity_list_callback>(
            client_dll.pattern("55 8B EC 51 8B 45 0C 53 8B D9 56 57 83 F8 FF 75 07")),
        [&](auto &&orig, auto handle_interface, auto handle) {
            // todo: work with this_ptr
            on_remove_entity(handle.index());
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

#if 1
    if (!hooks.disable_lazy())
        return false;
#else
    if (!hooks.disable())
        return false;
#endif

    return true;
}