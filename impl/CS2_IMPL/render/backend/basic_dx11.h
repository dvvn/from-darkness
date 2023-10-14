#pragma once
// ReSharper disable CppInconsistentNaming

struct ImDrawData;
struct IDirect3DDevice9;

// ReSharper restore CppInconsistentNaming

namespace fd
{
class basic_dx11_backend
{
  protected:
    ~basic_dx11_backend();
    basic_dx11_backend(IDirect3DDevice9* device);
    
  public:
    void new_frame();
    void render(ImDrawData* draw_data);
    void reset();
};
}