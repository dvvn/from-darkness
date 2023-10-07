#pragma once
#include "basic/directx9.h"
#include "functional/vtable.h"
#include "render/backend/basic_dx9.h"
#include "render/backend/basic_win32.h"
#include "render/frame.h"

#include <d3d9.h>

namespace fd
{
#ifdef _DEBUG
using native_dx9_vtable = vtable<IDirect3DDevice9>;
#else
using native_dx9_vtable = vtable<void>;
#endif

template <class Backend>
class hooked_directx9_reset final : public basic_directx9_hook
{
    Backend* render_;

  public:
    auto target() const
    {
        native_dx9_vtable const native(render_->native());
        return native[&IDirect3DDevice9::Reset];
    }

    hooked_directx9_reset(Backend* render)
        : render_(render)
    {
        static_assert(std::derived_from<Backend, basic_dx9_backend>);
    }

    HRESULT operator()(auto& original, D3DPRESENT_PARAMETERS* params) const
    {
        render_->reset();
        return original(params);
    }
};

template <class Frame>
class hooked_directx9_present final : public basic_directx9_hook
{
    Frame const* frame_;
    win32_backend_info* system_backend_info_;

  public:
    auto target() const
    {
        native_dx9_vtable const native(frame_->render_backend->native());
        return native[&IDirect3DDevice9::Present];
    }

    hooked_directx9_present(Frame const* render_frame, win32_backend_info* system_backend_info)
        : frame_(render_frame)
        , system_backend_info_(system_backend_info)
    {
    }

    HRESULT operator()(
        auto& original, //
        RECT const* source_rect, RECT const* dest_rect, HWND dest_window_override, RGNDATA const* dirty_region) const
    {
        if (!system_backend_info_->minimized())
            frame_->render();
        return original(source_rect, dest_rect, dest_window_override, dirty_region);
    }
};
} // namespace fd