#pragma once

#include "gui/present.h"
#include "gui/render/backend/native_dx11.h"
#include "gui/render/backend/native_win32.h"
#include "gui/render/context.h"

namespace fd::gui
{
struct native_data_dx11
{
    render_context ctx;
    native_win32_backend system_backend;
    native_dx11_backend render_backend;

    template <typename... T>
    void present(T*... data)
    {
        gui::present(&render_backend, &system_backend, &ctx, data...);
    }
};
} // namespace fd::gui