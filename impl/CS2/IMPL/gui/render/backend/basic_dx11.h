#pragma once
// ReSharper disable CppInconsistentNaming

struct ImDrawData;
struct ID3D11Device;
struct ID3D11DeviceContext;

// ReSharper restore CppInconsistentNaming

namespace fd::gui
{
class basic_dx11_backend
{
  protected:
    ~basic_dx11_backend();
    basic_dx11_backend(ID3D11Device* device, ID3D11DeviceContext* device_context);

  public:
    static void new_frame();
    static void render(ImDrawData* draw_data);
    static void reset();
};
}