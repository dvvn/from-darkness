#pragma once

#include "hook/proxy.h"

#include <d3d11.h>

namespace fd
{
namespace hooked
{
namespace DXGI_factory
{
template <class RenderBackend>
class create_swap_chain : public basic_hook_callback
{
    using native_function  = function_info<decltype(&IDXGIFactory::CreateSwapChain)>;
    using original_wrapped = object_froxy_for<native_function>::type;

    RenderBackend* render_backend_;

  public:
    create_swap_chain(RenderBackend* render_backend)
        : render_backend_(render_backend)
    {
    }

    HRESULT operator()(
        original_wrapped original,                                                       //
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
    using native_function  = function_info<decltype(&IDXGISwapChain::ResizeBuffers)>;
    using original_wrapped = object_froxy_for<native_function>::type;

    RenderBackend* render_backend_;

  public:
    resize_buffers(RenderBackend* render_backend)
        : render_backend_(render_backend)
    {
    }

    HRESULT operator()(
        original_wrapped original,                                                            //
        UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT flags) const //
    {
        auto result = original(buffer_count, width, height, new_format, flags);
        if (SUCCEEDED(result))
            render_backend_->resize();
        return result;
    }
};

template <class RenderFrame>
class present : public basic_hook_callback
{
    using native_function  = function_info<decltype(&IDXGISwapChain::Present)>;
    using original_wrapped = object_froxy_for<native_function>::type;

    RenderFrame render_frame_;

  public:
    present(RenderFrame render_frame)
        : render_frame_(render_frame)
    {
    }

    HRESULT operator()(
        original_wrapped original,            //
        UINT sync_interval, UINT flags) const //
    {
        render_frame_();
        return original(sync_interval, flags);
    }
};
} // namespace DXGI_swap_chain
} // namespace hooked
}