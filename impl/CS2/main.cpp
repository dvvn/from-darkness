#include "dll_context.h"
#include "debug/console.h"
#include "debug/log.h"
#include "functional/bind.h"
#include "functional/vtable.h"
#include "gui/menu.h"
#include "gui/menu/tab.h"
#include "hook/backend/minhook.h"
#include "hook/creator.h"
#include "hooked/directx11.h"
#include "hooked/winapi.h"
#include "render/backend/native_dx11.h"
#include "render/backend/native_win32.h"
#include "render/context.h"
#include "render/frame.h"

bool fd::run_context()
{
#ifdef _DEBUG
    system_console console;
    log_activator log_activator;
#endif

    using system_backend = native_win32_backend;
    using render_backend = native_dx11_backend;
    using hook_backend   = hook_backend_minhook;

    render_context render_ctx;
    system_backend system_bk;
    system_library_info render_system_lib(L"rendersystemdx11.dll");
    render_backend render_bk(render_system_lib);

    menu menu_holder(
        [] {
            using namespace fd::string_view_literals;
            menu_tab(
                "Tab1"sv,
                bind(menu_tab_item, "One"sv, bind_front(ImGui::TextUnformatted, "Text"sv)),
                bind(menu_tab_item, "Two"sv, bind_front(ImGui::TextUnformatted, "Text2"sv)));
            menu_tab(
                "Tab2"sv,
                bind(menu_tab_item, "__One"sv, bind_front(ImGui::TextUnformatted, "__Text"sv)),
                bind(menu_tab_item, "__Two"sv, bind_front(ImGui::TextUnformatted, "__Text2"sv)));
        },
        [] {
            if (!context_holder.resume())
            {
                assert(0 && "Unable to resume context");
                unreachable();
            }
        });
    auto const render_bk_data = render_bk.data();
    auto const system_bk_info = system_bk.info();

    hook_backend hook_bk;

    hooked::DXGI_factory::create_swap_chain const hk_create_swap_chain(&render_bk);
    create_hook(&hook_bk, vfunc(&IDXGIFactory::CreateSwapChain, render_bk_data->DXGI_factory()), &hk_create_swap_chain);
    hooked::DXGI_swap_chain::resize_buffers const hk_resize_buffers(&render_bk);
    create_hook(&hook_bk, vfunc(&IDXGISwapChain::ResizeBuffers, render_bk_data->swap_chain), &hk_resize_buffers);
    hooked::DXGI_swap_chain::present const hk_present(render_frame(&render_bk, &system_bk, &render_ctx, &menu_holder));
    create_hook(&hook_bk, vfunc(&IDXGISwapChain::Present, render_bk_data->swap_chain), &hk_present);
    hooked::winapi::wndproc const hk_wndproc(&system_bk);
    create_hook(&hook_bk, system_bk_info.proc(), &hk_wndproc);

    if (!hook_bk.enable())
        return false;

    if (!context_holder.pause())
    {
        assert(0 && "Unable to pause context");
        unreachable();
    }

    return true;
}
