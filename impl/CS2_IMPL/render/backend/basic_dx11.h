#pragma once
// ReSharper disable CppInconsistentNaming

struct ImDrawData;
struct ID3D11Device;
struct ID3D11DeviceContext;

// ReSharper restore CppInconsistentNaming

namespace fd
{
class basic_dx11_backend
{
  protected:
    ~basic_dx11_backend();
    basic_dx11_backend(ID3D11Device* device, ID3D11DeviceContext* device_context);
    
  public:
    void new_frame();
    void render(ImDrawData* draw_data);
    void reset();
};
}