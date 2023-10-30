#pragma once

#include "menu_example.h"
#include "debug/log.h"
#include "functional/bind.h"
#include "gui/menu.h"
#include "gui/menu/tab.h"
#include "render/backend/own_win32.h"
#include "render/context.h"
#include "render/frame.h"

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

    auto menu = make_menu_example(bind(&own_win32_backend::close, &system_bk));

    while (system_bk.update())
    {
        if (window.minimized())
            continue;
        render_bk.resize(window.size());
        render_frame(&render_bk, &system_bk, &render_ctx, &menu);
    }

    return true;
}
} // namespace fd
