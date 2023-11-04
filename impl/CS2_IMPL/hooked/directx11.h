#pragma once

#include "hook/proxy.h"

#include <d3d11.h>

namespace fd::hooked
{
namespace DXGI_factory
{
template <class RenderBackend>
class create_swap_chain : public basic_hook_callback
{
    using original_wrapped = object_froxy_for<decltype(&IDXGIFactory::CreateSwapChain), create_swap_chain>;

    RenderBackend* render_backend_;

  public:
    create_swap_chain(RenderBackend* render_backend)
        : render_backend_(render_backend)
    {
    }

    HRESULT operator()(
        original_wrapped const original,                                                 //
        IUnknown* device, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swap_chain) const //
    {
        render_backend_->reset();
        return original(device, desc, swap_chain);
    }
};
} // namespace DXGI_factory

namespace DXGI_swap_chain
{
template <class RenderBackend>
class resize_buffers : public basic_hook_callback
{
    using original_wrapped = object_froxy_for<decltype(&IDXGISwapChain::ResizeBuffers), resize_buffers>;

    RenderBackend* render_backend_;

  public:
    resize_buffers(RenderBackend* render_backend)
        : render_backend_(render_backend)
    {
    }

    HRESULT operator()(
        original_wrapped const original,                                                                                    //
        UINT const buffer_count, UINT const width, UINT const height, DXGI_FORMAT const new_format, UINT const flags) const //
    {
        auto const result = original(buffer_count, width, height, new_format, flags);
        if (SUCCEEDED(result))
            render_backend_->resize();
        return result;
    }
};

template <class RenderFrame>
class present : public basic_hook_callback
{
    using original_wrapped = object_froxy_for<decltype(&IDXGISwapChain::Present), present>;

    RenderFrame render_frame_;

  public:
    present(RenderFrame render_frame)
        : render_frame_(render_frame)
    {
    }

    HRESULT operator()(
        original_wrapped const original,                  //
        UINT const sync_interval, UINT const flags) const //
    {
        render_frame_();
        return original(sync_interval, flags);
    }
};
} // namespace DXGI_swap_chain
}