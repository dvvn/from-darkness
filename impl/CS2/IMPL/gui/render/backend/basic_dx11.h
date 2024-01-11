#pragma once

// ReSharper disable CppInconsistentNaming

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ImDrawData;

// ReSharper restore CppInconsistentNaming

namespace fd::gui
{
class basic_dx11_backend
{
  protected:
    ~basic_dx11_backend();
    basic_dx11_backend(ID3D11Device* device, ID3D11DeviceContext* device_context);

  public:
    using draw_data = ImDrawData;

    static void new_frame();
    static void render(draw_data* data);
    static void reset();
};
} // namespace fd::gui