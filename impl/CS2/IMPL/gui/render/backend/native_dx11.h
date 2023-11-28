#pragma once
#include "noncopyable.h"
#include "gui/render/backend/basic_dx11.h"
#include "utility/optional.h"
#include "winapi/com_ptr.h"

#include <d3d11.h>

namespace fd
{
struct native_library_info;

namespace gui
{
class native_dx11_device_data : public noncopyable
{
    win::com_ptr<IDXGISwapChain> swap_chain_;
    win::com_ptr<ID3D11Device> d3d_device_;
    win::com_ptr<ID3D11DeviceContext> device_context_;

    void setup_devie();

  public:
    using texture2d_ptr    = win::com_ptr<ID3D11Texture2D>;
    using DXGI_factory_ptr = win::com_ptr<IDXGIFactory>;

    native_dx11_device_data(IDXGISwapChain* sc);
    native_dx11_device_data(native_library_info info);

    native_dx11_device_data(native_dx11_device_data&& other) noexcept;
    native_dx11_device_data& operator=(native_dx11_device_data&& other) noexcept;

    IDXGISwapChain* swap_chain() const;
    ID3D11Device* d3d_device() const;
    ID3D11DeviceContext* device_context() const;

    // ReSharper disable once CppInconsistentNaming
    DXGI_factory_ptr DXGI_factory() const;
    texture2d_ptr back_buffer() const;

    void attach_swap_chain(IDXGISwapChain* swap_chain);
};

class basic_native_dx11_backend : basic_dx11_backend
{
    native_dx11_device_data data_;

    using back_buffer_ptr = native_dx11_device_data::texture2d_ptr;

    optional<D3D11_RENDER_TARGET_VIEW_DESC> render_target_desc_;
    win::com_ptr<ID3D11RenderTargetView> render_target_;

    bool init_render_target(back_buffer_ptr const& back_buffer);
    bool create_render_target(back_buffer_ptr const& back_buffer, D3D11_RENDER_TARGET_VIEW_DESC const* target_view_desc);
    bool create_render_target(back_buffer_ptr const& back_buffer);

  protected:
    basic_native_dx11_backend(native_dx11_device_data&& data);

  public:
    using basic_dx11_backend::new_frame;

    void render(ImDrawData* draw_data);
    void resize();
    void reset();

    native_dx11_device_data const& data() const;
    native_dx11_device_data& data();
};

template <class Context>
struct native_dx11_backend final : basic_native_dx11_backend, noncopyable
{
    native_dx11_backend(Context*, native_dx11_device_data data)
        : basic_native_dx11_backend{std::move(data)}
    {
    }
};
} // namespace gui
} // namespace fd