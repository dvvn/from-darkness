#include "directx9.h"
#include "functional/vtable.h"
#include "render/backend/basic_dx9.h"
#include "render/backend/basic_win32.h"
#include "render/frame.h"

#include <d3d9.h>

namespace fd
{
#ifdef _DEBUG
using native_vtable = vtable<IDirect3DDevice9>;
#else
using native_vtable = vtable<void>;
#endif

class hooked_directx9_reset final : public basic_directx9_hook
{
    basic_dx9_backend* render_;

  public:
    auto target() const
    {
        native_vtable const native(render_->native());
        return native[&IDirect3DDevice9::Reset];
    }

    hooked_directx9_reset(basic_dx9_backend* render)
        : render_(render)
    {
    }

    HRESULT operator()(auto& original, D3DPRESENT_PARAMETERS* params) const
    {
        render_->reset();
        return original(params);
    }
};

class hooked_directx9_present final : public basic_directx9_hook
{
    render_frame const* frame_;

    win32_backend_info system_backend_info_;

  public:
    auto target() const
    {
        native_vtable const native(frame_->render_backend->native());
        return native[&IDirect3DDevice9::Present];
    }

    hooked_directx9_present(render_frame const* render_frame)
        : frame_(render_frame)
    {
        frame_->system_backend->update(&system_backend_info_);
    }

    HRESULT operator()(
        auto& original, //
        RECT const* source_rect, RECT const* dest_rect, HWND dest_window_override, RGNDATA const* dirty_region) const
    {
        if (!system_backend_info_.minimized())
            frame_->render();
        return original(source_rect, dest_rect, dest_window_override, dirty_region);
    }
};

prepared_hook_data_full<basic_directx9_hook*> make_incomplete_object<hooked_directx9_reset>::operator()(basic_dx9_backend* backend) const
{
    return prepare_hook_wrapped<hooked_directx9_reset>(backend);
}

prepared_hook_data_full<basic_directx9_hook*> make_incomplete_object<hooked_directx9_present>::operator()(render_frame const* render) const
{
    return prepare_hook_wrapped<hooked_directx9_present>(render);
}
} // namespace fd