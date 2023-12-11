#include "debug/console.h"
#include "debug/log.h"
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
#include "library_info/construct.h"
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

    render_system_dx11_library_info const rendersystemdx11_dll;

    gui::render_context render_context;
    gui::native_win32_backend system_backend{&render_context};
    gui::native_dx11_backend render_backend{&render_context, rendersystemdx11_dll};

    auto menu = make_menu_example([=] {
        if (!this->resume())
            unreachable();
    });

    tier0_library_info const tier_dll;
    auto const cvar_system = tier_dll.interface().cvar_system();
    engine_lib const engine_dll;
    auto const engine_dll_interfaces = engine_dll.interface().known();
    schema_system_library_info const schemasystem_dll;
    auto const schema_system = schemasystem_dll.interface().schema_system();

    hook_backend_minhook hook_backend;

    auto const& render_data = render_backend.data();
    hooked::DXGI_factory::create_swap_chain hk_create_swap_chain{&render_backend};
    if (!create_hook(hook_backend, vfunc{&IDXGIFactory::CreateSwapChain, render_data.DXGI_factory()}, &hk_create_swap_chain))
        return false;
    hooked::DXGI_swap_chain::resize_buffers hk_resize_buffers{&render_backend};
    if (!create_hook(hook_backend, vfunc{&IDXGISwapChain::ResizeBuffers, render_data.swap_chain()}, &hk_resize_buffers))
        return false;
    hooked::DXGI_swap_chain::present hk_present{bind(gui::present, &render_backend, &system_backend, &render_context, &menu)};
    if (!create_hook(hook_backend, vfunc{&IDXGISwapChain::Present, render_data.swap_chain()}, &hk_present))
        return false;
    win::window_info const main_window{system_backend.window()};
    hooked::winapi::wndproc hk_wndproc{&system_backend};
    if (!create_hook(hook_backend, main_window.proc(), &hk_wndproc))
        return false;
    using cache_entity_fn = void* (native::game_resource_service::*)(native::entity_instance*, native::CBaseHandle);
    hooked::game_entity_system::on_add_entity<void> hk_on_add_entity;
    if (!create_hook(hook_backend, vfunc<cache_entity_fn>{14, engine_dll_interfaces.game_resource_service}, &hk_on_add_entity))
        return false;
    hooked::game_entity_system::on_remove_entity<void> hk_on_remove_entity;
    if (!create_hook(hook_backend, vfunc<cache_entity_fn>{15, engine_dll_interfaces.game_resource_service}, &hk_on_remove_entity))
        return false;

    if (!hook_backend.enable())
        return false;

    log("loaded!");

    if (!this->pause())
        return false;

    return true;
}
