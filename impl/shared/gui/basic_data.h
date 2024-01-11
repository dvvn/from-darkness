#pragma once

#include "gui/present.h"
#include "gui/render/context.h"

namespace fd::gui
{
template <class SystemBacked, class RenderBacked>
class basic_data
{
  public:
    render_context ctx;
    SystemBacked system_backend;
    RenderBacked render_backend;

    template <typename... T>
    void present(T*... data)
    {
        gui::present(&render_backend, &system_backend, &ctx, data...);
    }
};

template <class SystemBacked, class RenderBacked>
class basic_own_data
{
  public:
    render_context ctx;
    SystemBacked system_backend;
    RenderBacked render_backend;

    basic_own_data()
        : render_backend{system_backend.window().handle()}
    {
    }

    template <typename... T>
    void present(T*... data)
    {
        while (system_backend.update())
        {
            auto const window = system_backend.window();
            if (window.minimized())
                continue;
            render_backend.resize(window.size());
            gui::present(&render_backend, &system_backend, &ctx, data...);
        }
    }
};
} // namespace fd::gui