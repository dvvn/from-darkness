#include "directx9.h"
#include "functional/vtable.h"
#include "render/backend/basic_dx9.h"
#include "render/basic_system_backend.h"
#include "render/frame.h"

#include <d3d9.h>

namespace fd
{
class hooked_directx9_reset final : public basic_directx9_hook
{
    basic_dx9_backend *render_;

  public:
    auto target() const
    {
        vtable const native = render_->native();
        return native[&IDirect3DDevice9::Reset];
    }

    hooked_directx9_reset(basic_dx9_backend *render)
        : render_((render))
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
    render_frame frame_;

  public:
    auto target() const
    {
        vtable const native = static_cast<basic_dx9_backend *>(frame_.render_backend)->native();
        return native[&IDirect3DDevice9::Present];
    }

    hooked_directx9_present(render_frame const &render_frame)
        : frame_(render_frame)
    {
        assert(dynamic_cast<basic_dx9_backend *>(frame_.render_backend));
    }

    HRESULT operator()(
        auto &original, //
        RECT const *source_rect, RECT const *dest_rect, HWND dest_window_override, RGNDATA const *dirty_region) const
    {
        if (!frame_.system_backend->minimized())
            frame_.render();
        return original(source_rect, dest_rect, dest_window_override, dirty_region);
    }
};

FD_HOOK_IMPL(hooked_directx9_reset);
FD_HOOK_IMPL(hooked_directx9_present);
}