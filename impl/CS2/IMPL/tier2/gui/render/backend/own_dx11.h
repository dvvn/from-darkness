#pragma once
#include "tier1/noncopyable.h"
#include "tier1/winapi/com_ptr.h"
#include "tier1/winapi/window_info.h"
#include "tier2/gui/render/backend/basic_dx11.h"

#include <d3d11.h>

namespace FD_TIER(2)::gui
{
class own_dx11_backend_data : public noncopyable
{
    friend class own_dx11_backend;

    win::com_ptr<ID3D11Device> device_;
    win::com_ptr<ID3D11DeviceContext> device_context_;
    win::com_ptr<IDXGISwapChain> swap_chain_;
    win::com_ptr<ID3D11RenderTargetView> render_target_;

    template <UINT LevelsCount>
    HRESULT create_device_and_swap_chain(
        D3D_FEATURE_LEVEL const (&levels)[LevelsCount], DXGI_SWAP_CHAIN_DESC const* swap_chain_desc, //
        UINT const flags, D3D_DRIVER_TYPE const driver_type) noexcept
    {
        D3D_FEATURE_LEVEL feature_level;
        return D3D11CreateDeviceAndSwapChain(
            nullptr, driver_type, nullptr,                 //
            flags, levels, LevelsCount, D3D11_SDK_VERSION, //
            swap_chain_desc, &swap_chain_, &device_, &feature_level, &device_context_);
    }

  protected:
    own_dx11_backend_data(HWND hwnd);

    void create_render_target();
};

class own_dx11_backend final : own_dx11_backend_data, public basic_dx11_backend
{
    win::window_size_simple last_size_;

  public:
    own_dx11_backend(HWND hwnd);

    using basic_dx11_backend::new_frame;
    void render(ImDrawData* draw_data);
    void resize(win::window_size_simple const& size);
};
} // namespace FD_TIER(2)::gui