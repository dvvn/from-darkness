#include "backend.h"

#include <fd/gui/context.h>
#include <fd/gui/menu.h>
#include <fd/hooking/callback.h>
#include <fd/hooking/storage.h>

#include <imgui.h>

#include <spdlog/spdlog.h>

int main(int, char**)
{
    using namespace fd;
    backend_data backend;
    if (!backend.d3d)
        return EXIT_FAILURE;

    //---

    // log::init();
    spdlog::set_level(spdlog::level::debug);

#if defined(_DEBUG) && 0
    const default_assert_handler assertHandler([&](assert_data const& adata) { spdlog::critical(parse(adata)); });
#endif

    //----

    auto testMenu = menu(
        tab_bar(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
            "tab bar1",
#endif
            tab("tab1", [] { ImGui::TextUnformatted("hello"); }),
            tab("tab2", [] { ImGui::TextUnformatted("-->hello again"); })),
        tab_bar(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
            "tab bar2",
#endif
            tab("new tab", [] { ImGui::TextUnformatted("im here!"); }),
            tab("tab 3", [] { ImGui::TextUnformatted("yes!"); })));
    gui_context guiCtx(
        [&]
        {
            testMenu.render();
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
            ImGui::ShowDemoWindow();
#endif
        });

    if (!guiCtx.init({ false, backend.d3d, backend.hwnd }))
        return FALSE;

    auto hooks = hooks_storage(
        hook_callback_lazy(
            "WinAPI.WndProc",
            backend.info.lpfnWndProc,
            [&](auto orig, auto... args) -> LRESULT
            {
                using keys_return = basic_gui_context::keys_return;
                switch (guiCtx.process_keys(args...))
                {
                case keys_return::instant:
                    return TRUE;
                case keys_return::native:
                    return orig(args...);
                case keys_return::def:
                    return DefWindowProc(args...);
                default:
                    std::unreachable();
                }
            }),
        hook_callback_lazy(
            "IDirect3DDevice9::Reset",
            &IDirect3DDevice9::Reset,
            decay_fn(backend.d3d, 16),
            [&](auto orig, auto, auto... args)
            {
                guiCtx.release_textures();
                return orig(args...);
            }),
        hook_callback_lazy(
            "IDirect3DDevice9::Present",
            &IDirect3DDevice9::Present,
            decay_fn(backend.d3d, 17),
            [&](auto orig, auto thisPtr, auto... args)
            {
                guiCtx.render(thisPtr);
                return orig(args...);
            }));

    if (!hooks.enable())
        return EXIT_FAILURE;

    backend.run();
    return EXIT_SUCCESS;
}