#pragma once
#include "gui/render/backend/basic_dx11.h"
#include "winapi/com_ptr.h"
#include "winapi/window_info.h"
#include "noncopyable.h"
#ifdef _DEBUG
#include "library_info/literals.h"
#endif

#include <d3d11.h>

#include <cassert>

namespace fd::gui
{
class own_dx11_backend_data
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
    own_dx11_backend_data(HWND hwnd)
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

        auto res = DXGI_ERROR_UNSUPPORTED;
#ifdef _DEBUG
        if (L"d3d11_1sdklayers"_dll)
            res = create_device_and_swap_chain(feature_levels, &sc_desc, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE, D3D_DRIVER_TYPE_HARDWARE);
        if (res == DXGI_ERROR_UNSUPPORTED)
#endif
            res = create_device_and_swap_chain(feature_levels, &sc_desc, 0, D3D_DRIVER_TYPE_HARDWARE);
        if (res == DXGI_ERROR_UNSUPPORTED)
            res = create_device_and_swap_chain(feature_levels, &sc_desc, 0, D3D_DRIVER_TYPE_WARP);

        assert(res == S_OK);
        create_render_target();
    }

    void create_render_target()
    {
        win::com_ptr<ID3D11Texture2D> back_buffer;
        swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
        device_->CreateRenderTargetView(back_buffer.get(), nullptr, &render_target_);
    }
};

class own_dx11_backend final : own_dx11_backend_data, public basic_dx11_backend, public noncopyable
{
    using basic_dx11_backend::render;

    win::window_size_simple last_size_;

  public:
    own_dx11_backend(HWND hwnd)
        : own_dx11_backend_data{hwnd}
        , basic_dx11_backend{device_.get(), device_context_.get()}
        , last_size_{win::window_info{hwnd}.size()}
    {
    }

    void render(draw_data* data)
    {
        device_context_->OMSetRenderTargets(1, &render_target_, nullptr);
        constexpr FLOAT color[] = {0, 0, 0, 1};
        device_context_->ClearRenderTargetView(render_target_.get(), color);

        basic_dx11_backend::render(data);

        swap_chain_->Present(1, 0);
    }

    void resize(win::window_size_simple const& size)
    {
        if (last_size_ == size)
            return;
        last_size_ = size;

        //????????
        // render_target_.release();
        swap_chain_->ResizeBuffers(0, size.w, size.h, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        create_render_target();
    }
};
} // namespace fd::gui