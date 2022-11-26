#include "backend.h"

#include <fd/assert_impl.h>
#include <fd/functional.h>
#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hooked/hk_directx.h>
#include <fd/hooked/hk_winapi.h>
#include <fd/logger_impl.h>
#include <fd/system_console.h>

#include <imgui.h>

using namespace fd;

int main(int, char**)
{
    backend_data backend;
    if (!backend.d3d)
        return EXIT_FAILURE;

    default_assert_handler assert_callback;
    assert_handler = &assert_callback;

    system_console sys_console;

    default_logs_handler logs_callback;
    logger = &logs_callback;

    logs_callback.add([&](auto msg) {
        sys_console.write(msg);
    });

    assert_callback.add([&](auto& adata) {
        sys_console.write(parse_assert_data(adata));
    });

    gui::context_impl gui_ctx(&backend, false);
    gui::context = &gui_ctx;

    gui::menu_impl menu_ctx;
    gui::menu = &menu_ctx;

    gui::tab_bar test_tab_bar("test");
    gui::tab test_tab("test2");
    test_tab.store(bind_front(ImGui::Text, "Hello"));

    test_tab_bar.store(test_tab);
    menu_ctx.store(test_tab_bar);
    gui_ctx.store([&] {
        menu_ctx.render();
    });

    hook_holder all_hooks(hooked::d3d9_reset({ backend.d3d, 16 }), hooked::d3d9_present({ backend.d3d, 17 }), hooked::wndproc(backend.hwnd, backend.info.lpfnWndProc));

    return all_hooks.enable() ? (backend.run(), EXIT_SUCCESS) : EXIT_FAILURE;
}
