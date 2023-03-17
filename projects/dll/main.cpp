#include "console.h"

#include <fd/gui/context.h>
#include <fd/gui/menu.h>
#include <fd/hooking/callback.h>
#include <fd/hooking/storage.h>
#include <fd/library_info.h>
#include <fd/netvars/getter.h>
#include <fd/netvars/storage.h>
#include <fd/players/list.h>
#include <fd/utils/functional.h>

#include <fd/valve/base_client.h>
#include <fd/valve/client_entity_list.h>
#include <fd/valve/cs_player.h>
#include <fd/valve/engine_client.h>
#include <fd/valve/gui/surface.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <d3d9.h>
#include <windows.h>

static HMODULE _ModuleHandle;

static HANDLE _ThreadHandle;
static DWORD _ThreadId = 0;

namespace fd
{
class csgo_library_info_ex : public csgo_library_info
{
    void *create_interface_fn_;

    using csgo_library_info::find_interface;

  public:
    csgo_library_info_ex(library_info info)
        : csgo_library_info(info)
        , create_interface_fn_(info.find_export("CreateInterface"))
    {
    }

    auto find_interface(std::string_view name) const
    {
        return this->find_interface(create_interface_fn_, name);
    }
};

static void _unload()
{
    if (ResumeThread(_ThreadHandle) == -1)
        abort(); // WARNING!!!
}

static void _pause()
{
    if (SuspendThread(_ThreadHandle) == -1)
        abort(); // WARNING!!!
}

struct csgo_dlls
{
    csgo_library_info shader_api = find_library(L"shaderapidx9.dll");
    csgo_library_info_ex client  = find_library(L"client.dll");
    csgo_library_info_ex engine  = find_library(L"engine.dll");
    csgo_library_info_ex vgui    = find_library(L"vguimatsurface.dll");
};

class csgo_interfaces
{
    csgo_dlls const *lib_;

  public:
    csgo_interfaces(csgo_dlls const &libs)
        : lib_(&libs)
    {
    }

    IDirect3DDevice9 *&d3d = *(lib_->shader_api.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);

    valve::base_client *client        = lib_->client.find_interface("VClient");
    valve::engine_client *engine      = lib_->engine.find_interface("VEngineClient");
    valve::gui::surface *vgui_surface = lib_->vgui.find_interface("VGUI_Surface");
};

static netvars_storage *g_netvars;

size_t get_netvar_offset(std::string_view table, std::string_view name)
{
    return g_netvars->get_offset(table, name);
}

class netvars_data
{
    netvars_storage storage_;

#ifdef _DEBUG
    netvar_classes classes_;
    netvar_log log_;
#endif
  public:
    netvars_data()
    {
        g_netvars = &storage_;
    }

    void update(valve::client_class *cl_class)
    {
        storage_.iterate_client_class(cl_class);
    }

    void update(valve::data_map *map)
    {
        storage_.iterate_datamap(map);
    }

    void debug_update(valve::engine_client *engine)
    {
#ifdef _DEBUG
#ifdef FD_WORK_DIR
        classes_.dir.append(BOOST_STRINGIZE(FD_WORK_DIR)).append("netvars_generated").make_preferred();
#endif
        storage_.generate_classes(classes_);

        std::string_view native_str = (engine->GetProductVersionString());
        std::wstring buff;
        buff.reserve(native_str.size());
        for (auto c : native_str)
            buff += c == '.' ? '_' : c;
#ifdef FD_ROOT_DIR
        log_.dir.append(BOOST_STRINGIZE(FD_ROOT_DIR)).append(".dumps/netvars").make_preferred();
#endif
        log_.file.name      = std::move(buff);
        log_.file.extension = L".json";
        log_.indent         = 4;
        log_.filler         = ' ';

        storage_.log_netvars(log_);
#endif
    }
};

namespace valve
{
client_entity_list *entity_list;
}

template <typename Sample>
static Sample fn_sample(Sample, std::same_as<hidden_ptr> auto ptr)
{
    return ptr;
}

static DWORD WINAPI _context(void *) noexcept
{
    DWORD exit_code                = EXIT_FAILURE;
    invoke_on_destruct exit_helper = [&] {
        FreeLibraryAndExitThread(_ModuleHandle, exit_code);
    };

#ifdef _DEBUG
    auto console = console_holder(
        L"from-darkness debug console. " BOOST_STRINGIZE(__DATE__) " " BOOST_STRINGIZE(__TIME__));
#endif

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::warning);
#endif

    for (DWORD delay = 1000; !find_library(L"serverbrowser.dll"); delay += 1000)
        Sleep(delay);

    set_current_library(_ModuleHandle);

    csgo_dlls lib;
    // addToSafeList
    lib.client.find_signature("56 8B 71 3C B8").as_fn<void(__fastcall *)(HMODULE, void *)>(_ModuleHandle, nullptr);
    auto ifc = csgo_interfaces(lib);

    valve::cs_player *&local_player = *(lib.client.find_signature("A1 ? ? ? ? 89 45 BC 85 C0") + 1);

    netvars_data netvars;
    netvars.update(ifc.client->GetAllClasses());
    if (local_player)
    {
        netvars.update(local_player->GetDataDescMap());
        netvars.update(local_player->GetPredictionDescMap());
    }
    else
    {
        valve::cs_player *vtable = lib.client.find_vtable("C_CSPlayer");
        netvars.update(vtable->GetDataDescMap());
        netvars.update(vtable->GetPredictionDescMap());
    }
    netvars.debug_update(ifc.engine);

    init_netvars<valve::cs_player>();

    auto hack_menu = menu(tab_bar(tab("tab", [] {
        ImGui::TextUnformatted("test");
        if (ImGui::Button("Unload"))
            _unload();
    })));

    auto gui_ctx = gui_context([&] {
        [[maybe_unused]] auto visible = hack_menu.render();
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
        if (visible)
            ImGui::ShowDemoWindow();
#endif
    });

    D3DDEVICE_CREATION_PARAMETERS d3d_params;
    if (FAILED(ifc.d3d->GetCreationParameters(&d3d_params)))
        abort(); // WARNING!!!

    if (!gui_ctx.init({ false, ifc.d3d, d3d_params.hFocusWindow }))
        return FALSE;
    invoke_on_destruct gui_ctx_protector = [&] {
        if (!ifc.d3d)
            gui_ctx.detach();
    };

    players_list players;

    auto all_hooks = hooks_storage(
        hook_callback_args(
            "WinAPI.WndProc",
            fn_sample<WNDPROC>(GetWindowLongPtrW(d3d_params.hFocusWindow, GWLP_WNDPROC)),
            [&](auto orig, HWND hwnd, auto... args) -> LRESULT {
                assert(hwnd == d3d_params.hFocusWindow);
                using keys_return = basic_gui_context::keys_return;
                switch (gui_ctx.process_keys(hwnd, args...))
                {
                case keys_return::instant:
                    return TRUE;
                case keys_return::native:
                    return orig(hwnd, args...);
                case keys_return::def:
                    return DefWindowProcW(hwnd, args...);
                default:
                    std::unreachable();
                }
            }),
        hook_callback_args(
            "IDirect3DDevice9::Reset",
            fn_sample(&IDirect3DDevice9::Reset, vfunc(ifc.d3d, 16)),
            [&](auto orig, auto, auto... args) {
                gui_ctx.release_textures();
                return orig(args...);
            }),
        hook_callback_args(
            "IDirect3DDevice9::Present",
            fn_sample(&IDirect3DDevice9::Present, vfunc(ifc.d3d, 17)),
            [&](auto orig, auto this_ptr, auto... args) {
                gui_ctx.render(this_ptr);
                return orig(args...);
            }),
        hook_callback_args(
            "VGUI.ISurface::LockCursor",
            fn_sample(&valve::gui::surface::LockCursor, vfunc(ifc.vgui_surface, 67)),
            [&](auto orig, auto this_ptr) {
                if (hack_menu.visible() && !this_ptr->IsCursorVisible() /*&& ifc.engine->IsInGame()*/)
                {
                    this_ptr->UnlockCursor();
                    return;
                }
                orig();
            }),
        hook_callback_args(
            "IBaseClientDll::CreateMove",
            fn_sample(&valve::base_client::CreateMove, vfunc(ifc.client, 22)),
            [&](auto orig, auto this_ptr, auto... args) {
                //
                orig(args...);
            }),
        hook_callback_args(
            "CClientEntityList::OnAddEntity",
            fn_sample(
                &valve::client_entity_list::OnAddEntity,
                lib.client.find_signature("?????")),
            [&](auto orig, auto this_ptr, auto ent, auto handle) {
                orig(ent, handle);
                players.on_add_entity(this_ptr, handle);
            }),
        hook_callback_args(
            "CClientEntityList::OnRemoveEntity",
            fn_sample(
                &valve::client_entity_list::OnRemoveEntity,
                lib.client.find_signature("?????")),
            [&](auto orig, auto this_ptr, auto ent, auto handle) {
                players.on_remove_entity(this_ptr, handle);
                orig(ent, handle);
            })
        //
    );

    if (!all_hooks.enable())
        return FALSE;

#if 0
    if (!libs.wait(L"serverbrowser.dll"))
        _exit_fail();
#endif

    _pause();

    if (!all_hooks.disable())
        return FALSE;

    Sleep(100);

    exit_code = EXIT_SUCCESS;
    return TRUE;
}
} // namespace fd

// ReSharper disable once CppInconsistentNaming
BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD reason, LPVOID /*reserved*/)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        if (!DisableThreadLibraryCalls(moduleHandle))
            return FALSE;
        _ModuleHandle = moduleHandle;
        _ThreadHandle = CreateThread(nullptr, 0, fd::_context, nullptr, 0, &_ThreadId);
        if (!_ThreadId)
            return FALSE;
        break;
    }
    case DLL_PROCESS_DETACH: {
        // if (_ThreadId && WaitForSingleObject(_ThreadHandle, INFINITE) == WAIT_FAILED)
        //     return FALSE;
        break;
    }
    }

    return TRUE;
}