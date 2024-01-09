#pragma once

#include "gui/present.h"
#include "gui/render/backend/own_dx11.h"
#include "gui/render/backend/own_win32.h"
#include "gui/render/context.h"
#include "winapi/window_info.h"

namespace fd::gui
{
struct own_data_dx11
{
    render_context ctx;
    own_win32_backend system_backend;
    own_dx11_backend render_backend;

    own_data_dx11()
        : render_backend{system_backend.window()}
    {
    }

    template <typename... T>
    void present(T*... data)
    {
        win::window_info const wnd_info{system_backend.window()};
        while (system_backend.update())
        {
            if (wnd_info.minimized())
                continue;
            render_backend.resize(wnd_info.size());
            gui::present(&render_backend, &system_backend, &ctx, data...);
        }
    }
};
} // namespace fd::gui