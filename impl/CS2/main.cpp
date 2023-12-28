#include "debug/console.h"
#include "debug/log.h"
#include "entity_cache/holder.h"
#include "functional/bind.h"
#include "functional/vtable.h"
#include "gui/present.h"
#include "gui/render/backend/native_dx11.h"
#include "gui/render/backend/native_win32.h"
#include "gui/render/context.h"
#include "hook/backend/minhook.h"
#include "hook/creator.h"
#include "hooked/directx11.h"
#include "hooked/game_entity_system.h"
#include "hooked/winapi.h"
#include "library_info/engine.h"
#include "library_info/render_system_dx11.h"
#include "library_info/schema_system.h"
#include "library_info/tier0.h"
#include "winapi/window_info.h"
//
#include "dll_context.h"
#include "menu_example.h"

bool fd::context::run()
{
#ifdef _DEBUG
    system_console console;
    if (!console.exists())
        return false;
    log_activator log_activator;
#endif

    gui::render_context render_context;
    gui::native_win32_backend system_backend{&render_context};
    gui::native_dx11_backend render_backend{&render_context, "rendersystemdx11"_dll.pattern().DXGI_swap_chain()};
    auto menu = make_menu_example([=] {
        if (!this->resume())
            unreachable();
    });

    entity_cache ent_cache;

    auto const tier0_dll                = "tier0"_dll;
    auto const [cvar_system]            = tier0_dll.interface();
    auto const engine_dll               = "engine2"_dll;
    auto const [engine, game_resources] = engine_dll.interface();
    auto const schemasystem_dll         = "schemasystem"_dll;
    auto const [schema_system]          = schemasystem_dll.interface();

    hook_backend_minhook hook_backend;
    create_hook_helper const hook_creator{&hook_backend};

    auto const& render_data = render_backend.data();
    hooked::DXGI_factory::create_swap_chain hk_create_swap_chain{&render_backend};
    if (!hook_creator(vfunc{&IDXGIFactory::CreateSwapChain, render_data.DXGI_factory()}, &hk_create_swap_chain))
        return false;
    hooked::DXGI_swap_chain::resize_buffers hk_resize_buffers{&render_backend};
    if (!hook_creator(vfunc{&IDXGISwapChain::ResizeBuffers, render_data.swap_chain()}, &hk_resize_buffers))
        return false;
    hooked::DXGI_swap_chain::present hk_present{bind(gui::present, &render_backend, &system_backend, &render_context, &menu)};
    if (!hook_creator(vfunc{&IDXGISwapChain::Present, render_data.swap_chain()}, &hk_present))
        return false;
    win::window_info const main_window{system_backend.window()};
    hooked::winapi::wndproc hk_wndproc{&system_backend};
    if (!hook_creator(main_window.proc(), &hk_wndproc))
        return false;
    using cache_entity_vfunc = vfunc<void* (native::game_resource_service::*)(native::entity_instance*, native::CBaseHandle)>;
    hooked::game_entity_system::on_add_entity hk_on_add_entity{&ent_cache};
    if (!hook_creator(cache_entity_vfunc{14, game_resources}, &hk_on_add_entity))
        return false;
    hooked::game_entity_system::on_remove_entity hk_on_remove_entity{&ent_cache};
    if (!hook_creator(cache_entity_vfunc{15, game_resources}, &hk_on_remove_entity))
        return false;

    if (!hook_backend.enable())
        return false;

    log("loaded!");

    if (!this->pause())
        return false;

    return true;
}
