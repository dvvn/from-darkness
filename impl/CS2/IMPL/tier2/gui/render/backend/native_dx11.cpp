#include "tier1/memory/address.h"
#include "tier1/pattern/make.h"
#include "tier2/gui/render/backend/native_dx11.h"
#include "tier2/library_info.h"
#include "tier2/native/dx11_swap_chain.h"

#include <d3d11.h>

#include <algorithm>
#include <cassert>

namespace FD_TIER(2)::gui
{
native_dx11_device_data::native_dx11_device_data(IDXGISwapChain* sc)
    : swap_chain_(sc)
{
    setup_devie();
}

static IDXGISwapChain* find_native_swap_chain(library_info info)
{
    assert(info.name() == L"rendersystemdx11.dll");
    auto const addr = info.find("66 0F 7F 05 ? ? ? ? 66 0F 7F 0D ? ? ? ? 48 89 35"_pat);
    assert(addr != nullptr); // nullptr only for range style
    auto const abs_addr = resolve_relative_address(addr, 0x4, 0x8);
    assert(abs_addr != nullptr);
    auto const native_swap_chain = **static_cast<native::dx11_swap_chain***>(abs_addr);
    assert(native_swap_chain != nullptr);

    return native_swap_chain->pDXGISwapChain;
}

native_dx11_device_data::native_dx11_device_data(native_library_info info)
    : native_dx11_device_data{find_native_swap_chain(info)}
{
}

native_dx11_device_data::native_dx11_device_data(native_dx11_device_data&& other) noexcept
    : swap_chain_{std::move(other.swap_chain_)}
    , d3d_device_{std::move(other.d3d_device_)}
    , device_context_{std::move(other.device_context_)}
{
}

native_dx11_device_data& native_dx11_device_data::operator=(native_dx11_device_data&& other) noexcept
{
    using std::swap;
    swap(swap_chain_, other.swap_chain_);
    swap(d3d_device_, other.d3d_device_);
    swap(device_context_, other.device_context_);
    return *this;
}

IDXGISwapChain* native_dx11_device_data::swap_chain() const
{
    return swap_chain_.get();
}

ID3D11Device* native_dx11_device_data::d3d_device() const
{
    return d3d_device_.get();
}

ID3D11DeviceContext* native_dx11_device_data::device_context() const
{
    return device_context_.get();
}

auto native_dx11_device_data::DXGI_factory() const -> DXGI_factory_ptr
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

auto native_dx11_device_data::back_buffer() const -> texture2d_ptr
{
    ID3D11Texture2D* back_buffer;
    swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    return {back_buffer, std::in_place};
}

void native_dx11_device_data::attach_swap_chain(IDXGISwapChain* swap_chain)
{
    swap_chain_.attach(swap_chain);
}

void native_dx11_device_data::setup_devie()
{
    swap_chain_->GetDevice(IID_PPV_ARGS(&d3d_device_));
    d3d_device_->GetImmediateContext(&device_context_);
}

bool basic_native_dx11_backend::init_render_target(back_buffer_ptr const& back_buffer)
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
        render_target_desc_.emplace(target_view_desc);
        return true;
    }
    return create_render_target(back_buffer, nullptr);
}

bool basic_native_dx11_backend::create_render_target(back_buffer_ptr const& back_buffer, D3D11_RENDER_TARGET_VIEW_DESC const* target_view_desc)
{
    auto const result = data_.d3d_device()->CreateRenderTargetView(back_buffer.get(), target_view_desc, &render_target_);
    return SUCCEEDED(result);
}

bool basic_native_dx11_backend::create_render_target(back_buffer_ptr const& back_buffer)
{
    return create_render_target(back_buffer, render_target_desc_ ? render_target_desc_.operator->() : nullptr);
}

basic_native_dx11_backend::basic_native_dx11_backend(native_dx11_device_data&& data)
    : basic_dx11_backend(data.d3d_device(), data.device_context())
    , data_(std::move(data))
{
    auto const bb = data_.back_buffer();
    if (!init_render_target(bb))
        assert(0 && "failed to create render target view");
}

// basic_native_dx11_backend::basic_native_dx11_backend(system_library_info info)
//     : basic_native_dx11_backend(native_dx11_device_data(info))
//{
// }

void basic_native_dx11_backend::render(ImDrawData* draw_data)
{
    data_.device_context()->OMSetRenderTargets(1, &render_target_, nullptr);
    basic_dx11_backend::render(draw_data);
}

void basic_native_dx11_backend::resize()
{
    auto const bb = data_.back_buffer();
    if (!create_render_target(bb))
        assert(0 && "failed to create render target view");
}

void basic_native_dx11_backend::reset()
{
    render_target_.reset(); // release?
    // basic_dx11_backend::reset(); //ImGui_ImplDX11_InvalidateDeviceObjects
}

native_dx11_device_data const& basic_native_dx11_backend::data() const
{
    return data_;
}

native_dx11_device_data& basic_native_dx11_backend::data()
{
    return data_;
}
}