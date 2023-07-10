#pragma once
#include "internal/d3d9.h"
#include "render/basic_context.h"
#include "render/basic_render_backend.h"
#include "render/basic_system_backend.h"

namespace ImGui
{
void ShowDemoWindow(bool *p_open);
}

namespace fd
{
class hooked_dx9_reset final
{
    basic_render_backend *render_ = nullptr;

  public:
    hooked_dx9_reset(basic_render_backend *render)
        : render_(render)
    {
    }

    HRESULT operator()(auto &original, D3DPRESENT_PARAMETERS *params) const
    {
        render_->reset();
        return original(params);
    }
};

class hooked_dx9_present final
{
    basic_render_backend *render_;
    basic_system_backend *system_;
    basic_render_context *context_;

    void run() const
    {
        system_->new_frame();
        render_->new_frame();

        context_->begin_scene();
        {
            // only as example
            ImGui::ShowDemoWindow(0);
        }
        context_->end_scene();

        render_->render(context_->data());
    }

  public:
    hooked_dx9_present(basic_render_backend *render, basic_system_backend *system, basic_render_context *context)
        : render_(render)
        , system_(system)
        , context_(context)
    {
    }

    HRESULT operator()(
        auto &original,
        RECT const *source_rect,
        RECT const *dest_rect,
        HWND dest_window_override,
        RGNDATA const *dirty_region)
    {
        if (!system_->minimized())
            run();
        return original(source_rect, dest_rect, dest_window_override, dirty_region);
    }
};
}