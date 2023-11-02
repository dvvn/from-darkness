#include "gui/render/backend/own_dx11.h"

#include <cassert>

namespace fd::gui
{
own_dx11_backend_data::own_dx11_backend_data(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC sc_desc;
#ifndef _DEBUG
    memset(&sc_desc, 0, sizeof(DXGI_SWAP_CHAIN_DESC));
#endif
    sc_desc.BufferCount                        = 2;
    sc_desc.BufferDesc.Width                   = 0;
    sc_desc.BufferDesc.Height                  = 0;
    sc_desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sc_desc.BufferDesc.RefreshRate.Numerator   = 60;
    sc_desc.BufferDesc.RefreshRate.Denominator = 1;
    sc_desc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sc_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_desc.OutputWindow                       = hwnd;
    sc_desc.SampleDesc.Count                   = 1;
    sc_desc.SampleDesc.Quality                 = 0;
    sc_desc.Windowed                           = TRUE;
    sc_desc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    constexpr D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT res;
#ifdef _DEBUG
    res = create_device_and_swap_chain(feature_levels, &sc_desc, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE, D3D_DRIVER_TYPE_HARDWARE);
    if (res == DXGI_ERROR_UNSUPPORTED)
#endif
        res = create_device_and_swap_chain(feature_levels, &sc_desc, 0, D3D_DRIVER_TYPE_HARDWARE);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = create_device_and_swap_chain(feature_levels, &sc_desc, 0, D3D_DRIVER_TYPE_WARP);

    assert(res == S_OK);
    create_render_target();
}

void own_dx11_backend_data::create_render_target()
{
    win::com_ptr<ID3D11Texture2D> back_buffer;
    swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    device_->CreateRenderTargetView(back_buffer, nullptr, &render_target_);
}

own_dx11_backend::own_dx11_backend(HWND hwnd)
    : own_dx11_backend_data(hwnd)
    , basic_dx11_backend(device_, device_context_)
    // ReSharper disable once CppPossiblyUnintendedObjectSlicing
    , last_size_(win::window_info(hwnd).size())
{
}

void own_dx11_backend::render(ImDrawData* draw_data)
{
    device_context_->OMSetRenderTargets(1, &render_target_, nullptr);
    constexpr FLOAT color[] = {0, 0, 0, 1};
    device_context_->ClearRenderTargetView(render_target_, color);

    basic_dx11_backend::render(draw_data);

    swap_chain_->Present(1, 0);
}

void own_dx11_backend::resize(win::window_size_simple const& size)
{
    if (last_size_ == size)
        return;
    last_size_ = size;

    //????????
    // render_target_.release();
    swap_chain_->ResizeBuffers(0, size.w, size.h, DXGI_FORMAT_UNKNOWN, 0);
    create_render_target();
}
} // namespace fd::gui