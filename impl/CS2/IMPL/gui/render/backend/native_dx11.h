#pragma once
#include "gui/render/backend/basic_dx11.h"
#include "library_info/render_system_dx11.h"
#include "winapi/com_ptr.h"
#include "noncopyable.h"
#include "optional.h"

#include <d3d11.h>

#include <algorithm>
#include <cassert>

namespace fd
{
namespace gui
{
class native_dx11_device_data : public noncopyable
{
    win::com_ptr<IDXGISwapChain> swap_chain_;
    win::com_ptr<ID3D11Device> d3d_device_;
    win::com_ptr<ID3D11DeviceContext> device_context_;

    void setup_devie()
    {
        swap_chain_->GetDevice(IID_PPV_ARGS(&d3d_device_));
        d3d_device_->GetImmediateContext(&device_context_);
    }

  public:
    using texture2d_ptr    = win::com_ptr<ID3D11Texture2D>;
    using DXGI_factory_ptr = win::com_ptr<IDXGIFactory>;

    using source_library = render_system_dx11_library_info;

    native_dx11_device_data(IDXGISwapChain* sc)
        : swap_chain_(sc)
    {
        setup_devie();
    }

    native_dx11_device_data(source_library source)
        : native_dx11_device_data{source.pattern().DXGI_swap_chain()}
    {
    }

    native_dx11_device_data(native_dx11_device_data&& other) noexcept
        : swap_chain_{std::move(other.swap_chain_)}
        , d3d_device_{std::move(other.d3d_device_)}
        , device_context_{std::move(other.device_context_)}
    {
    }

    native_dx11_device_data& operator=(native_dx11_device_data&& other) noexcept
    {
        using std::swap;
        swap(swap_chain_, other.swap_chain_);
        swap(d3d_device_, other.d3d_device_);
        swap(device_context_, other.device_context_);
        return *this;
    }

    IDXGISwapChain* swap_chain() const
    {
        return swap_chain_.get();
    }

    ID3D11Device* d3d_device() const
    {
        return d3d_device_.get();
    }

    ID3D11DeviceContext* device_context() const
    {
        return device_context_.get();
    }

    // ReSharper disable once CppInconsistentNaming
    DXGI_factory_ptr DXGI_factory() const
    {
        win::com_ptr<IDXGIDevice> device;
        d3d_device_->QueryInterface(IID_PPV_ARGS(&device));
        win::com_ptr<IDXGIAdapter> adapter;
        // device->GetParent(IID_PPV_ARGS(&adapter));
        device->GetAdapter(&adapter);
        DXGI_factory_ptr factory;
        adapter->GetParent(IID_PPV_ARGS(&factory));
        return factory;
    }

    texture2d_ptr back_buffer() const
    {
        ID3D11Texture2D* back_buffer;
        swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
        return {back_buffer, std::in_place};
    }

    void attach_swap_chain(IDXGISwapChain* swap_chain)
    {
        swap_chain_.attach(swap_chain);
    }
};

class basic_native_dx11_backend : basic_dx11_backend
{
    native_dx11_device_data data_;
    optional<D3D11_RENDER_TARGET_VIEW_DESC> render_target_desc_;
    win::com_ptr<ID3D11RenderTargetView> render_target_;

    using back_buffer_ptr = native_dx11_device_data::texture2d_ptr;

    bool init_render_target()
    {
        return init_render_target(data_.back_buffer());
    }

    bool init_render_target(back_buffer_ptr const& back_buffer)
    {
        DXGI_SWAP_CHAIN_DESC swap_chain_desc;
        data_.swap_chain()->GetDesc(&swap_chain_desc);

        D3D11_RENDER_TARGET_VIEW_DESC target_view_desc;
#ifndef _DEBUG
        std::fill(reinterpret_cast<uint8_t*>(&target_view_desc), reinterpret_cast<uint8_t*>(&target_view_desc + 1), 0);
#endif
        if (swap_chain_desc.BufferDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
            target_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        else
            target_view_desc.Format = swap_chain_desc.BufferDesc.Format;
        target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        if (create_render_target(back_buffer, &target_view_desc))
        {
            render_target_desc_.emplace(std::move(target_view_desc));
            return true;
        }
        return create_render_target(back_buffer, nullptr);
    }

    bool create_render_target(back_buffer_ptr const& back_buffer, D3D11_RENDER_TARGET_VIEW_DESC const* target_view_desc)
    {
        auto const result = data_.d3d_device()->CreateRenderTargetView(back_buffer.get(), target_view_desc, &render_target_);
        return SUCCEEDED(result);
    }

    bool create_render_target()
    {
        return create_render_target(data_.back_buffer());
    }

    bool create_render_target(back_buffer_ptr const& back_buffer)
    {
        return create_render_target(back_buffer, render_target_desc_ ? render_target_desc_.operator->() : nullptr);
    }

  protected:
    basic_native_dx11_backend(native_dx11_device_data&& data)
        : basic_dx11_backend(data.d3d_device(), data.device_context())
        , data_{std::move(data)}
    {
        if (!init_render_target())
            assert(0 && "failed to create render target view");
    }

  public:
    using basic_dx11_backend::new_frame;

    void render(draw_data* data)
    {
        data_.device_context()->OMSetRenderTargets(1, &render_target_, nullptr);
        basic_dx11_backend::render(data);
    }

    void resize()
    {
        if (!create_render_target())
            assert(0 && "failed to create render target view");
    }

    void reset()
    {
        render_target_.reset(); // release?
        // basic_dx11_backend::reset(); //ImGui_ImplDX11_InvalidateDeviceObjects
    }

    native_dx11_device_data const& data() const&
    {
        return data_;
    }

    native_dx11_device_data& data() &
    {
        return data_;
    }
};

template <class Context>
struct native_dx11_backend final : basic_native_dx11_backend, noncopyable
{
    native_dx11_backend(Context*, native_dx11_device_data&& data)
        : basic_native_dx11_backend{std::move(data)}
    {
    }
};
} // namespace gui
} // namespace fd