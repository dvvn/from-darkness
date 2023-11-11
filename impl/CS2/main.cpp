#include "dll_context.h"
#include "menu_example.h"
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
#include "hooked/winapi.h"

bool fd::run_context()
{
#ifdef _DEBUG
    system_console console;
    if (!console.exists())
        return false;
    log_activator log_activator;
#endif

    gui::render_context render_context;
    gui::native_win32_backend system_backend;
    gui::native_dx11_backend render_backend(L"rendersystemdx11"_dll);

    auto menu = gui::make_menu_example([] {
        if (!context_holder.resume())
            unreachable();
    });

    auto const render_data = render_backend.data();
    win::window_info const main_window(system_backend.window());

    hook_backend_minhook hook_backend;

    hooked::DXGI_factory::create_swap_chain hk_create_swap_chain(&render_backend);
    if (!create_hook(&hook_backend, vfunc(&IDXGIFactory::CreateSwapChain, render_data->DXGI_factory()), &hk_create_swap_chain))
        return false;
    hooked::DXGI_swap_chain::resize_buffers hk_resize_buffers(&render_backend);
    if (!create_hook(&hook_backend, vfunc(&IDXGISwapChain::ResizeBuffers, render_data->swap_chain()), &hk_resize_buffers))
        return false;
    hooked::DXGI_swap_chain::present hk_present(bind(gui::present, &render_backend, &system_backend, &render_context, &menu));
    if (!create_hook(&hook_backend, vfunc(&IDXGISwapChain::Present, render_data->swap_chain()), &hk_present))
        return false;
    hooked::winapi::wndproc hk_wndproc(&system_backend);
    if (!create_hook(&hook_backend, main_window.proc(), &hk_wndproc))
        return false;

    if (!hook_backend.enable())
        return false;
    if (!context_holder.pause())
        return false;

    return true;
}
