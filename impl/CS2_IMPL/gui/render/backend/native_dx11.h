#pragma once
#include "noncopyable.h"
#include "optional.h"
#include "gui/render/backend/basic_dx11.h"
#include "library_info/system.h"
#include "winapi/com_ptr.h"

#include <d3d11.h>

namespace fd::gui
{
struct native_dx11_device_data
{
    win::com_ptr<IDXGISwapChain> swap_chain;
    win::com_ptr<ID3D11Device> d3d_device;
    win::com_ptr<ID3D11DeviceContext> device_context;

  private:
    void setup_devie();

  public:
    native_dx11_device_data(IDXGISwapChain* sc);
    native_dx11_device_data(system_library_info info);

    // ReSharper disable once CppInconsistentNaming
    win::com_ptr<IDXGIFactory> DXGI_factory() const;
};

class native_dx11_backend final : basic_dx11_backend, public noncopyable
{
    native_dx11_device_data data_;

    optional<D3D11_RENDER_TARGET_VIEW_DESC> render_target_desc_;
    win::com_ptr<ID3D11RenderTargetView> render_target_;

    win::com_ptr<ID3D11Texture2D> back_buffer() const;

    bool init_render_target(ID3D11Texture2D* back_buffer);
    bool create_render_target(ID3D11Texture2D* back_buffer, D3D11_RENDER_TARGET_VIEW_DESC const* target_view_desc);
    bool create_render_target(ID3D11Texture2D* back_buffer);

  public:
    native_dx11_backend(native_dx11_device_data data);
    native_dx11_backend(system_library_info info);

    using basic_dx11_backend::new_frame;

    void render(ImDrawData* draw_data);
    void resize();
    void reset();

    native_dx11_device_data const* data() const;
};

} // namespace fd::gui