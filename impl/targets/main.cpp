#include "own_backend.h"

#include <fd/hooking/callback.h>
#include <fd/lazy_invoke.h>
#include <fd/library_info.h>
#include <fd/logging/init.h>
#include <fd/netvars/impl/storage.h>
#include <fd/render/context_init.h>
#include <fd/render/context_update.h>
#include <fd/render/frame.h>
#include <fd/vfunc.h>
//
#include <fd/valve/client.h>
#include <fd/valve/client_side/cs_player.h>
#include <fd/valve/client_side/engine.h>
#include <fd/valve/client_side/entity_list.h>
#include <fd/valve/gui/surface.h>

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

template <typename T>
// ReSharper disable once CppMismatchedClassTags
struct vtable<T *> : vtable<T>
{
    using vtable<T>::vtable;

    template <std::convertible_to<T *> Q>
    vtable(Q val)
        : vtable<T>(val)
    {
    }
};
} // namespace fd

static bool context(HINSTANCE self_handle);

// #define _WINDLL

#ifdef _WINDLL
namespace fd
{
template <>
struct cast_helper<render_backend>
{
    render_backend operator()(to<uintptr_t> val) const
    {
        return **reinterpret_cast<render_backend **>(val + 1);
    }
};

class netvars_holder
{
    netvars_storage storage_;

#ifdef _DEBUG
    netvar_classes classes_;
    netvar_log log_;
#endif

  public:
    netvars_holder(
        valve::client_side::cs_player *player,
        valve::client_class *cl_class,
        valve::client_side::engine *engine)
    {
        storage_.process(cl_class);
        storage_.process(player->GetDataDescMap());
        storage_.process(player->GetPredictionDescMap());

#ifdef _DEBUG
#ifdef FD_WORK_DIR
        classes_.dir.append(BOOST_STRINGIZE(FD_WORK_DIR)).make_preferred().append("netvars_generated");
#endif
        storage_.write(classes_);

        std::string_view native_str = engine->GetProductVersionString();
        log_.file.name.reserve(native_str.size());
        for (auto c : native_str)
            log_.file.name.push_back(c == '.' ? '_' : c);
#ifdef FD_ROOT_DIR
        log_.dir.append(BOOST_STRINGIZE(FD_ROOT_DIR)).make_preferred().append(".out").append("netvars_dump");
#endif
        log_.file.extension = L".txt";
        log_.indent         = 4;
        log_.filler         = ' ';

        storage_.write(log_);
#endif
    }
};
} // namespace fd

static HANDLE thread;
static DWORD thread_id;

static DWORD WINAPI context_proxy(LPVOID ptr)
{
    if (!context(static_cast<HINSTANCE>(ptr)))
        FreeLibraryAndExitThread(static_cast<HMODULE>(ptr), EXIT_FAILURE);
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
#define USE_OWN_RENDER_BACKEND

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    return context(GetModuleHandle(nullptr)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif

static bool context([[maybe_unused]] HINSTANCE self_handle)
{
    using namespace fd;

    logger_registrar::start();
    [[maybe_unused]] //
    invoke_on_destruct stop_logging = logger_registrar::stop;

    struct
    {
        render_context ctx;
        vtable<render_backend> vtable;
    } render;

    HWND window;
    WNDPROC window_proc;

#ifdef USE_OWN_RENDER_BACKEND
    auto own_render = own_render_backend(L"Unnamed", self_handle);
    if (!own_render.initialized())
        return false;

    render.vtable = own_render.device;
    window        = own_render.hwnd;
    window_proc   = own_render.info.lpfnWndProc;
#else
    library_info shader_api_dll                   = L"shaderapidx9.dll";
    to<cast_helper<render_backend>> packed_render = shader_api_dll.find_pattern("A1 ? ? ? ? 50 8B 08 FF 51 0C");
    D3DDEVICE_CREATION_PARAMETERS creation_parameters;
    if (FAILED(packed_render->GetCreationParameters(&creation_parameters)))
        return false;
    to<WNDPROC> wnd_proc = GetWindowLongPtr(creation_parameters.hFocusWindow, GWLP_WNDPROC);

    render.vtable = packed_render;
    window        = creation_parameters.hFocusWindow;
    window_proc   = wnd_proc;
#endif

    if (!init(&render.ctx, render.vtable, window))
        return false;

#ifdef _WINDLL
    game_library_info_ex client_dll = (L"client.dll");
    game_library_info_ex engine_dll = (L"engine.dll");
    game_library_info_ex vgui_dll   = (L"vguimatsurface.dll");

    vtable<valve::client> client_ifc           = client_dll.find_interface("VClient");
    valve::client_side::engine *engine_ifc     = engine_dll.find_interface("VEngineClient");
    vtable<valve::gui::surface> vgui_surface   = vgui_dll.find_interface("VGUI_Surface");
    valve::client_side::entity_list *ents_list = client_dll.find_interface("VClientEntityList");

    // todo: check if ingame and use exisiting player
    vtable<valve::client_side::cs_player> player_vtable = client_dll.find_vtable("C_CSPlayer");

    auto netvars = netvars_holder(player_vtable, client_ifc->GetAllClasses(), engine_ifc);
#endif

    basic_hook *hooks[] = {
        make_hook_callback(
            "WinAPI.WndProc",
            window_proc,
            [&](auto orig, auto hwnd, auto... args) -> LRESULT {
                assert(window == hwnd);
                process_message_result pmr;
                process_message(&render.ctx, args..., &pmr);
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
            magic_cast(render.vtable.func(16), &IDirect3DDevice9::Reset),
            [&](auto orig, auto this_ptr, auto... args) {
                assert(render.vtable == this_ptr);
                reset(&render.ctx);
                return orig(args...);
            }),
        make_hook_callback(
            "IDirect3DDevice9::Present",
            magic_cast(render.vtable.func(17), &IDirect3DDevice9::Present),
            [&](auto orig, auto this_ptr, auto... args) {
                assert(render.vtable == this_ptr);
                if (auto frame = render_frame(&render.ctx))
                {
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
                    ImGui::ShowDemoWindow();
#endif
                }
                return orig(args...);
            }),
#ifdef _WINDLL
        make_hook_callback(
            "VGUI.ISurface::LockCursor",
            magic_cast(vgui_surface.func(67), &valve::gui::surface::LockCursor),
            [&](auto orig, auto this_ptr) {
                // if (hack_menu.visible() && !this_ptr->IsCursorVisible() /*&& ifc.engine->IsInGame()*/)
                //{
                //     this_ptr->UnlockCursor();
                //     return;
                // }
                orig();
            }),
        make_hook_callback(
            "CHLClient::CreateMove",
            to<void (valve::client::*)(int, int, bool)>(client_ifc.func(22)),
            [&](auto orig, auto this_ptr, auto... args) {
                //
                orig(args...);
            }),
        make_hook_callback(
            "CClientEntityList::OnAddEntity",
            to<void(__thiscall *)(void *, valve::client_side::entity *, valve::handle)>(
                client_dll.find_pattern("55 8B EC 51 8B 45 0C 53 56 8B F1 57")),
            [&](auto orig, auto this_ptr, auto ent, auto handle) {
                orig(ent, handle);
                // todo: work with this_ptr
                // players.on_add_entity(ifc.ents_list, handle);
            }),
        make_hook_callback(
            "CClientEntityList::OnRemoveEntity",
            to<void(__thiscall *)(void *, valve::client_side::entity *, valve::handle)>(
                client_dll.find_pattern("55 8B EC 51 8B 45 0C 53 8B D9 56 57 83 F8 FF 75 07")),
            [&](auto orig, auto this_ptr, auto ent, auto handle) {
                // todo: work with this_ptr
                // players.on_remove_entity(ifc.ents_list, handle);
                orig(ent, handle);
            })
#endif
    };

    [[maybe_unused]] //
    invoke_on_destruct hooks_destroyer = [&] {
        destroy_hooks(hooks);
    };

    if (!enable_hooks(hooks))
        return false;

#ifdef USE_OWN_RENDER_BACKEND
    if (!own_render.run())
        return false;
#endif

#ifdef _WINDLL
    if (!disable_hooks(hooks))
        return false;
#endif

    return true;
}