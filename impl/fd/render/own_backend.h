#pragma once

#include "fd/comptr.h"
#include "fd/core.h"

#include <Windows.h>
#include <d3d9.h>
#include <tchar.h>

namespace fd
{
struct native_render_backend;
struct own_d3d_device;

class own_window_info : public noncopyable
{
    WNDCLASSEX info_;
    HWND hwnd_;

  public:
    ~own_window_info();
    own_window_info(LPCTSTR name, HMODULE handle, HWND parent);

    void bind(own_d3d_device *device);
    void show();

    HWND get() const;
    WNDPROC proc() const;
};

struct own_d3d_device
{
    using value_type = IDirect3DDevice9;
    using pointer    = value_type *;

  private:
    comptr<IDirect3D9> d3d_;
    comptr<IDirect3DDevice9> device_;
    D3DPRESENT_PARAMETERS params_;

  public:
    own_d3d_device(HWND hwnd);

    bool resize(UINT w, UINT h);
    void reset();

    operator pointer() const;
    pointer get() const;
    pointer operator->() const;
};

class own_render_backend : public noncopyable
{
    own_window_info window_;
    own_d3d_device device_;

  public:
    own_render_backend(LPCTSTR name, HMODULE handle = GetModuleHandle(nullptr), HWND parent = nullptr);

    bool run();
    bool stop();

    native_render_backend backend() const;
    HWND window() const;
    WNDPROC window_proc() const;
    WNDPROC default_window_proc() const;
};
} // namespace fd