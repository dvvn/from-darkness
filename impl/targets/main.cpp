#include "own_backend.h"

#include <fd/hooking/callback.h>
#include <fd/lazy_invoke.h>
#include <fd/library_info.h>
#include <fd/logging/core.h>
#include <fd/logging/default.h>
#include <fd/netvars/core.h>
#include <fd/players/list.h>
#include <fd/render/context.h>
#include <fd/vfunc.h>

#include <windows.h>
#include <d3d9.h>

#include <algorithm>
#include <functional>

// ReSharper disable once CppInconsistentNaming
namespace ImGui
{
// ReSharper disable once CppInconsistentNaming
extern void ShowDemoWindow(bool *open = nullptr);
} // namespace ImGui

static bool context(HINSTANCE self_handle) noexcept;

#ifdef FD_SHARED_LIB
static HANDLE thread;
static DWORD thread_id;

static DWORD WINAPI context_proxy(LPVOID ptr)
{
    auto result = context(static_cast<HINSTANCE>(ptr));
    FreeLibraryAndExitThread(static_cast<HMODULE>(ptr), result ? EXIT_SUCCESS : EXIT_FAILURE);
    // return TRUE;
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
#define USE_OWN_RENDER_BACKEND

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    return context(GetModuleHandle(nullptr)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif

#define SECOND_ARG(_A_, _B_, ...) _B_
#define DUMMY_VAR                 BOOST_JOIN(dummy, __COUNTER__)

#define MAKE_DESTRUCTOR(_FN_, ...) \
    [[maybe_unused]]               \
    invoke_on_destruct SECOND_ARG(unused, ##__VA_ARGS__, const DUMMY_VAR) = _FN_;

static bool context(HINSTANCE self_handle) noexcept
{
    (void)self_handle;

    using namespace fd;

    init_logging();
    MAKE_DESTRUCTOR(stop_logging);

    vtable<IDirect3DDevice9> render_vtable;
    HWND window;
    WNDPROC window_proc;

#ifdef USE_OWN_RENDER_BACKEND
    auto own_render = own_render_backend(L"Unnamed", self_handle);
    if (!own_render.initialized())
        return false;

    render_vtable = own_render.device;
    window        = own_render.hwnd;
    window_proc   = own_render.info.lpfnWndProc;
#else
    library_info shader_api_dll = L"shaderapidx9.dll";
    auto d3d9_device            = [&] {
        uintptr_t val = shader_api_dll.find_pattern("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        return **reinterpret_cast<IDirect3DDevice9 ***>(val + 1);
    }();
    D3DDEVICE_CREATION_PARAMETERS creation_parameters;
    if (FAILED(d3d9_device->GetCreationParameters(&creation_parameters)))
        return false;
    to<WNDPROC> wnd_proc = GetWindowLongPtr(creation_parameters.hFocusWindow, GWLP_WNDPROC);

    render_vtable = d3d9_device;
    window        = creation_parameters.hFocusWindow;
    window_proc   = wnd_proc;
#endif

    if (!create_render_context(window, render_vtable.instance()))
        return false;
    MAKE_DESTRUCTOR(destroy_render_context);

#ifdef FD_SHARED_LIB
    game_library_info_ex client_dll = L"client.dll";
    game_library_info_ex engine_dll = L"engine.dll";
    game_library_info_ex vgui_dll   = L"vguimatsurface.dll";

    vtable client_interface      = client_dll.find_interface("VClient");
    // vtable engine_interface    = engine_dll.find_interface("VEngineClient");
    vtable vgui_interface        = vgui_dll.find_interface("VGUI_Surface");
    vtable entity_list_interface = client_dll.find_interface("VClientEntityList");

    // todo: check if ingame and use exisiting player
    auto player_vtable = client_dll.find_vtable("C_CSPlayer");

    store_netvars(client_interface);
    store_extra_netvars(player_vtable);
    store_custom_netvars(client_dll);
#endif
    hook_id hooks[] = {
        make_hook_callback(
            "WinAPI.WndProc",
            window_proc,
            [](auto orig, auto... args) -> LRESULT {
                render_message_result pmr;
                process_render_message(args..., &pmr);
                switch (pmr)
                {
                case render_message_result::idle:
                    return orig(args...);
                case render_message_result::updated:
                    return DefWindowProc(args...);
                case render_message_result::locked:
                    return TRUE;
                default:
                    std::unreachable();
                }
            }),
#ifndef USE_OWN_RENDER_BACKEND
        make_hook_callback(
            "IDirect3DDevice9::Release",
            render_vtable[&IDirect3DDevice9::Release],
            [&](auto orig) -> ULONG {
                auto refs = orig();
                if (refs == 0)
                    render_backend_detach();
                return refs;
            }),
#endif
        make_hook_callback(
            "IDirect3DDevice9::Reset",
            render_vtable[&IDirect3DDevice9::Reset],
            [&](auto orig, auto... args) {
                reset_render_context();
                return orig(args...);
            }),
        make_hook_callback(
            "IDirect3DDevice9::Present",
            render_vtable[&IDirect3DDevice9::Present],
            [&](auto orig, auto... args) {
                if (auto frame = render_frame())
                {
                    // #ifndef IMGUI_DISABLE_DEMO_WINDOWS
                    ImGui::ShowDemoWindow();
                    // #endif
                }
                return orig(args...);
            }),
#ifdef FD_SHARED_LIB
        make_hook_callback(
            "VGUI.ISurface::LockCursor",
            to<void(__thiscall *)(void *)>(vgui_interface[67]),
            [&](auto orig) {
                // if (hack_menu.visible() && !this_ptr->IsCursorVisible() /*&& ifc.engine->IsInGame()*/)
                //{
                //     this_ptr->UnlockCursor();
                //     return;
                // }
                orig();
            }),
        make_hook_callback(
            "CHLClient::CreateMove",
            to<void(__thiscall *)(void *, int, int, bool)>(client_interface[22]),
            [&](auto orig, auto... args) {
                //
                orig(args...);
            }),
        make_hook_callback(
            "CClientEntityList::OnAddEntity",
            to<void(__thiscall *)(void *, void *, valve::entity_handle)>(
                client_dll.find_pattern("55 8B EC 51 8B 45 0C 53 56 8B F1 57")),
            [&](auto orig, auto handle_interface, auto handle) {
                orig(handle_interface, handle);
                // todo: work with this_ptr
                on_add_entity(entity_list_interface, handle);
            }),
        make_hook_callback(
            "CClientEntityList::OnRemoveEntity",
            to<void(__thiscall *)(void *, void *, valve::entity_handle)>(
                client_dll.find_pattern("55 8B EC 51 8B 45 0C 53 8B D9 56 57 83 F8 FF 75 07")),
            [&](auto orig, auto handle_interface, auto handle) {
                // todo: work with this_ptr
                on_remove_entity(entity_list_interface, handle);
                orig(handle_interface, handle);
            })
#endif
    };
    MAKE_DESTRUCTOR(disable_hooks, hooks_guard);

#ifdef _DEBUG
    if (!std::all_of(std::begin(hooks), std::end(hooks), enable_hook_lazy))
        return false;
    if (!apply_lazy_hooks())
        return false;
#else
    if (std::find(std::begin(hooks), std::end(hooks), 0) != std::end(hooks))
        return false;
    if (!enable_hooks())
        return false;
#endif

#ifdef USE_OWN_RENDER_BACKEND
    if (!own_render.run())
        return false;
#endif

#ifdef _DEBUG
    if (!std::all_of(std::rbegin(hooks), std::rend(hooks), disable_hook_lazy))
        return false;
    if (!apply_lazy_hooks())
        return false;
    hooks_guard = nullptr;
#elif defined(FD_SHARED_LIB) && 0
    if (!disable_hooks())
        return false;
    hooks_guard = nullptr;
#endif

    return true;
}