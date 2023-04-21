#pragma once

#include <fd/comptr.h>

#include <Windows.h>
#include <d3d9.h>

namespace fd
{
class d3d_device9
{
    comptr<IDirect3D9> d3d_;
    comptr<IDirect3DDevice9> device_;
    D3DPRESENT_PARAMETERS params_;

  public:
    d3d_device9();

    bool attach(HWND hwnd);
    bool resize(UINT w, UINT h);
    void reset();

    operator IDirect3DDevice9 *() const;
    IDirect3DDevice9 *get() const;
    IDirect3DDevice9 *operator->() const;
};

struct own_render_backend
{
    d3d_device9 d3d;
    HWND hwnd;
    WNDCLASSEX info;

    ~own_render_backend();
    own_render_backend(LPCTSTR name, HMODULE handle);

    own_render_backend(own_render_backend const &) = delete;

    bool run();
};
} // namespace fd
