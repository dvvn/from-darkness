#pragma once
#include "comptr.h"
#include "noncopyable.h"
#include "optional.h"
#include "gui/render/backend/basic_dx11.h"
#include "library_info/system.h"

#include <d3d11.h>

namespace fd
{
struct native_dx11_device_data
{
    comptr<IDXGISwapChain> swap_chain;
    comptr<ID3D11Device> d3d_device;
    comptr<ID3D11DeviceContext> device_context;

  private:
    void setup_devie();

  public:
    native_dx11_device_data(IDXGISwapChain* sc);
    native_dx11_device_data(system_library_info info);

    // ReSharper disable once CppInconsistentNaming
    comptr<IDXGIFactory> DXGI_factory() const;
};

class native_dx11_backend final : basic_dx11_backend, public noncopyable
{
    native_dx11_device_data data_;

    optional<D3D11_RENDER_TARGET_VIEW_DESC> render_target_desc_;
    comptr<ID3D11RenderTargetView> render_target_;

    comptr<ID3D11Texture2D> back_buffer() const;

    bool init_render_target(ID3D11Texture2D* back_buffer);
    bool create_render_target(ID3D11Texture2D* back_buffer, D3D11_RENDER_TARGET_VIEW_DESC const* target_view_desc);
    bool create_render_target(ID3D11Texture2D* back_buffer);

  public:
    native_dx11_backend(native_dx11_device_data data);
    // native_dx11_backend(system_library_info info);

    using basic_dx11_backend::new_frame;

    void render(ImDrawData* draw_data);
    void resize();
    void reset();

    native_dx11_device_data const* data() const;
};

} // namespace fd