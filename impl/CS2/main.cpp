#include "dll_context.h"
#include "debug/console.h"
#include "debug/log.h"
#include "functional/bind.h"
#include "gui/menu.h"
#include "gui/menu/tab.h"
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

    render_context render_ctx;
    system_backend system_bk;
    system_library_info render_system_lib(L"rendersystemdx11.dll");
    render_backend render_bk(render_system_lib);
    using namespace fd::string_view_literals;

    menu menu_holder(
        bind(menu_tab, "Tab1"sv, bind_front(menu_tab_item, "One"sv, bind(ImGui::TextUnformatted, "Text"sv))), //
        [] {});

    render_frame const render(&render_bk, &system_bk, &render_ctx, &menu_holder);

    return true;
}
