#include "algorithm/find.h"
#include "memory/address.h"
#include "memory/pattern.h"
#include "native/dx11_swap_chain.h"
#include "render/backend/native_dx11.h"

#include <d3d11.h>

#include <algorithm>
#include <cassert>

namespace fd
{
native_dx11_device_data::native_dx11_device_data(IDXGISwapChain* sc)
    : swap_chain(sc)
{
    setup_devie();
}

native_dx11_device_data::native_dx11_device_data(system_library_info info)
{
    assert(info.name() == L"rendersystemdx11.dll");
    auto const addr = find(begin(info), end(info), "66 0F 7F 05 ? ? ? ? 66 0F 7F 0D ? ? ? ? 48 89 35"_pat);
    assert(addr != nullptr);
    auto const abs_addr = resolve_relative_address(addr, 0x4, 0x8);
    assert(abs_addr != nullptr);
    auto const native_swap_chain = **static_cast<dx11_swap_chain***>(abs_addr);
    assert(native_swap_chain != nullptr);

    std::construct_at(std::addressof(swap_chain), native_swap_chain->pDXGISwapChain);
    setup_devie();
}

comptr<IDXGIFactory> native_dx11_device_data::DXGI_factory() const
{
    comptr<IDXGIDevice> device;
    d3d_device->QueryInterface(IID_PPV_ARGS(&device));
    comptr<IDXGIAdapter> adapter;
    // device->GetParent(IID_PPV_ARGS(&adapter));
    device->GetAdapter(&adapter);
    comptr<IDXGIFactory> factory;
    adapter->GetParent(IID_PPV_ARGS(&factory));
    return (factory);
}

void native_dx11_device_data::setup_devie()
{
    swap_chain->GetDevice(IID_PPV_ARGS(&d3d_device));
    d3d_device->GetImmediateContext(&device_context);
}

comptr<ID3D11Texture2D> native_dx11_backend::back_buffer() const
{
    ID3D11Texture2D* back_buffer;
    data_.swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    return {back_buffer, std::in_place};
}

bool native_dx11_backend::init_render_target(ID3D11Texture2D* back_buffer)
{
    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    data_.swap_chain->GetDesc(&swap_chain_desc);

    D3D11_RENDER_TARGET_VIEW_DESC target_view_desc;
#ifndef _DEBUG
    std::fill(reinterpret_cast<uint8_t*>(&target_view_desc), reinterpret_cast<uint8_t*>(&target_view_desc + 1), 0);
#endif
    switch (swap_chain_desc.BufferDesc.Format)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        target_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    default:
        target_view_desc.Format = swap_chain_desc.BufferDesc.Format;
        break;
    }
    constexpr D3D11_RTV_DIMENSION view_demensions[] = {D3D11_RTV_DIMENSION_TEXTURE2D, D3D11_RTV_DIMENSION_TEXTURE2DMS};
    for (auto const dem : view_demensions)
    {
        target_view_desc.ViewDimension = dem;
        if (create_render_target(back_buffer, &target_view_desc))
        {
            render_target_desc_.emplace(target_view_desc);
            return true;
        }
    }
    return create_render_target(back_buffer, nullptr);
}

bool native_dx11_backend::create_render_target(ID3D11Texture2D* back_buffer, D3D11_RENDER_TARGET_VIEW_DESC const* target_view_desc)
{
    auto const result = data_.d3d_device->CreateRenderTargetView(back_buffer, target_view_desc, &render_target_);
    return SUCCEEDED(result);
}

bool native_dx11_backend::create_render_target(ID3D11Texture2D* back_buffer)
{
    return create_render_target(back_buffer, render_target_desc_ ? render_target_desc_.operator->() : nullptr);
}

native_dx11_backend::native_dx11_backend(native_dx11_device_data data)
    : basic_dx11_backend(data.d3d_device, data.device_context)
    , data_(std::move(data))
{
    auto const bb = back_buffer();
    if (!init_render_target(bb))
        assert(0 && "failed to create render target view");
}

// native_dx11_backend::native_dx11_backend(system_library_info info)
//     : native_dx11_backend(native_dx11_device_data(info))
//{
// }

void native_dx11_backend::render(ImDrawData* draw_data)
{
    data_.device_context->OMSetRenderTargets(1, &render_target_, nullptr);
    basic_dx11_backend::render(draw_data);
}

void native_dx11_backend::resize()
{
    auto const bb = back_buffer();
    if (!create_render_target(bb))
        assert(0 && "failed to create render target view");
}

void native_dx11_backend::reset()
{
    render_target_.reset(); // release?
    // basic_dx11_backend::reset(); //ImGui_ImplDX11_InvalidateDeviceObjects
}

native_dx11_device_data const* native_dx11_backend::data() const
{
    return &data_;
}
}