#pragma once
#include "internal/d3d9.h"
#include "render/basic_frame.h"
#include "render/basic_render_backend.h"

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
    basic_render_frame *render_frame_;

  public:
    hooked_dx9_present(basic_render_frame *render_frame)
        : render_frame_(render_frame)
    {
    }

    HRESULT operator()(
        auto &original,
        RECT const *source_rect,
        RECT const *dest_rect,
        HWND dest_window_override,
        RGNDATA const *dirty_region
    )
    {
        render_frame_->render();
        return original(source_rect, dest_rect, dest_window_override, dirty_region);
    }
};
}