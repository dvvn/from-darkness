#pragma once

#include "menu.h"
#include "menu/tab.h"
//
#include "debug/log.h"
#include "functional/bind.h"
#include "render/backend/own_win32.h"
#include "render/context.h"
#include "render/frame.h"

namespace fd
{
template <class RenderBackend>
bool menu_sample()
{
#ifdef _DEBUG
    log_activator log_activator;
#endif

    using render_backend = RenderBackend;

    render_context render_ctx;
    own_win32_backend system_bk;
    auto const window = system_bk.info();
    render_backend render_bk(window.id);
    using namespace fd::string_view_literals;
    menu menu_holder(
        bind(menu_tab, "Tab1"sv, menu_tab_item("One", bind(ImGui::TextUnformatted, "Text"sv))), //
        bind(&own_win32_backend::close, &system_bk));

    render_frame const render_frame(&render_bk, &system_bk, &render_ctx, &menu_holder);

    while (system_bk.update())
    {
        if (window.minimized())
            continue;
        render_bk.resize(window.size());
        render_frame.render();
    }

    return true;
}
} // namespace fd
