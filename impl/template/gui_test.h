#pragma once

#include "menu_example.h"
#include "debug/log.h"
#include "gui/present.h"
#include "gui/render/backend/own_win32.h"
#include "gui/render/context.h"

namespace fd
{
template <class RenderBackend>
bool gui_test()
{
#ifdef _DEBUG
    log_activator log_activator;
#endif

    using render_backend = RenderBackend;

    render_context render_ctx;
    own_win32_backend system_bk;
    auto const window = system_bk.info();
    render_backend render_bk(window.id);

    auto menu = make_menu_example([&system_bk] {
        system_bk.close();
    });

    while (system_bk.update())
    {
        if (window.minimized())
            continue;
        render_bk.resize(window.size());
        present_gui(&render_bk, &system_bk, &render_ctx, &menu);
    }

    return true;
}
} // namespace fd
