#include "gui/present.h"
#include "gui/render/backend/own_dx11.h"
#include "gui/render/backend/own_win32.h"
#include "gui/render/context.h"
#include "winapi/window_info.h"
#include "menu_example.h"
//
#include "exe_context.h"

bool fd::context_holder(context* const ctx)
{
    auto logger = ctx->make_debug_logger();

    gui::render_context render_ctx;
    gui::own_win32_backend system_bk;
    win::window_info const window{system_bk.window()};
    gui::own_dx11_backend render_bk{window.handle()};

    auto menu = make_menu_example([&system_bk] {
        system_bk.close();
    });

    logger("Loaded");

    while (system_bk.update())
    {
        if (window.minimized())
            continue;
        render_bk.resize(window.size());
        gui::present(&render_bk, &system_bk, &render_ctx, &menu);
    }

    return true;
}
