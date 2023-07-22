#include "directx9.h"
#include "render/basic_frame.h"
#include "render/basic_render_backend.h"

#include <d3d9.h>

namespace fd
{
class hooked_directx9_reset final : public basic_directx9_hook
{
    basic_render_backend *render_ = nullptr;

  public:
    using function_type = decltype(&IDirect3DDevice9::Reset);

    hooked_directx9_reset(basic_render_backend *render)
        : render_(render)
    {
    }

    HRESULT operator()(auto &original, D3DPRESENT_PARAMETERS *params) const
    {
        render_->reset();
        return original(params);
    }
};

class hooked_directx9_present final : public basic_directx9_hook
{
    basic_render_frame *render_frame_;

  public:
    using function_type = decltype(&IDirect3DDevice9::Present);

    hooked_directx9_present(basic_render_frame *render_frame)
        : render_frame_(render_frame)
    {
    }

    HRESULT operator()(
        auto &original, //
        RECT const *source_rect, RECT const *dest_rect, HWND dest_window_override, RGNDATA const *dirty_region)
    {
        render_frame_->render();
        return original(source_rect, dest_rect, dest_window_override, dirty_region);
    }
};

FD_HOOK_IMPL(hooked_directx9_reset);
FD_HOOK_IMPL(hooked_directx9_present);
}