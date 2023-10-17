#pragma once

#include "comptr.h"
#include "noncopyable.h"
#include "render/backend/basic_dx11.h"
#include "render/backend/basic_win32.h"

#include <d3d11.h>

namespace fd
{
class own_dx11_backend_data : public noncopyable
{
    friend class own_dx11_backend;

    comptr<ID3D11Device> device_;
    comptr<ID3D11DeviceContext> device_context_;
    comptr<IDXGISwapChain> swap_chain_;
    comptr<ID3D11RenderTargetView> render_target_;

    template <UINT LevelsCount>
    HRESULT create_device_and_swap_chain(
        D3D_FEATURE_LEVEL const (&levels)[LevelsCount], DXGI_SWAP_CHAIN_DESC const* swap_chain_desc, //
        UINT const flags, D3D_DRIVER_TYPE const driver_type)
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
    simple_win32_window_size last_size_;

  public:
    own_dx11_backend(HWND hwnd);

    using basic_dx11_backend::new_frame;
    void render(ImDrawData* draw_data);
    void resize(simple_win32_window_size const& size);
};
} // namespace fd