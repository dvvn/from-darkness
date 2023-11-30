#pragma once

#include "tier1/winapi/window_info.h"
#include "tier2/debug/log.h"
#include "tier2/gui/present.h"
#include "tier2/gui/render/backend/own_win32.h"
#include "tier2/gui/render/context.h"
#include "menu_example.h"

namespace FD_TIER(3)
{
template <class RenderBackend>
bool run_test()
{
#ifdef _DEBUG
    log_activator log_activator;
#endif

    using render_backend = RenderBackend;
    using gui::own_win32_backend;
    using gui::render_context;

    render_context render_ctx;
    own_win32_backend system_bk;
    win::window_info const window{system_bk.window()};
    render_backend render_bk{window.handle()};

    auto menu = make_menu_example([&system_bk] {
        system_bk.close();
    });

    while (system_bk.update())
    {
        if (window.minimized())
            continue;
        render_bk.resize(window.size());
        gui::present(&render_bk, &system_bk, &render_ctx, &menu);
    }

    return true;
}
} // namespace FD_TIER(3)
