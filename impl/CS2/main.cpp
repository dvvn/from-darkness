#include "tier1/functional/bind.h"
#include "tier1/functional/vtable.h"
#include "tier1/winapi/window_info.h"
#include "tier2/debug/console.h"
#include "tier2/debug/log.h"
#include "tier2/gui/present.h"
#include "tier2/gui/render/backend/native_dx11.h"
#include "tier2/gui/render/backend/native_win32.h"
#include "tier2/gui/render/context.h"
#include "tier2/hook/backend/minhook.h"
#include "tier2/hook/creator.h"
#include "tier2/library_info/native.h"
#include "tier2/native/cvar.hpp"
#include "tier2/native/engine_client.hpp"
#include "tier2/native/interface_register.h"
#include "tier2/native/schema_system.hpp"
#include "tier3/hooked/directx11.h"
#include "tier3/hooked/winapi.h"
#include "dll_context.h"
#include "menu_example.h"

bool fd::run_context()
{
#ifdef _DEBUG
    system_console console;
    if (!console.exists())
        return false;
    log_activator log_activator;
#endif

    gui::render_context render_context;
    gui::native_win32_backend system_backend{&render_context};
    gui::native_dx11_backend render_backend{&render_context, "rendersystemdx11"_dlln};

    auto menu = make_menu_example([] {
        if (!context_holder.resume())
            unreachable();
    });

    auto const tier_dll                        = "tier0"_dlln;
    native::cvar_system const* cvar_system     = safe_cast_from(get(tier_dll.root_interface(), "VEngineCvar"));
    auto const engine_dll                      = "engine2"_dlln;
    native::engine_client const* engine        = safe_cast_from(get(engine_dll.root_interface(), "Source2EngineToClient"));
    auto const schemasystem_dll                = "schemasystem"_dlln;
    native::schema_system const* schema_system = safe_cast_from(get(schemasystem_dll.root_interface(), "SchemaSystem"));

    auto const& render_data = render_backend.data();
    win::window_info const main_window{system_backend.window()};

    hook_backend_minhook hook_backend;

    hooked::DXGI_factory::create_swap_chain hk_create_swap_chain{&render_backend};
    if (!create_hook(hook_backend, vfunc{&IDXGIFactory::CreateSwapChain, render_data.DXGI_factory()}, &hk_create_swap_chain))
        return false;
    hooked::DXGI_swap_chain::resize_buffers hk_resize_buffers{&render_backend};
    if (!create_hook(hook_backend, vfunc{&IDXGISwapChain::ResizeBuffers, render_data.swap_chain()}, &hk_resize_buffers))
        return false;
    hooked::DXGI_swap_chain::present hk_present{bind(gui::present, &render_backend, &system_backend, &render_context, &menu)};
    if (!create_hook(hook_backend, vfunc{&IDXGISwapChain::Present, render_data.swap_chain()}, &hk_present))
        return false;
    hooked::winapi::wndproc hk_wndproc{&system_backend};
    if (!create_hook(hook_backend, main_window.proc(), &hk_wndproc))
        return false;

    if (!hook_backend.enable())
        return false;

    log("loaded!");

    if (!context_holder.pause())
        return false;

    return true;
}
