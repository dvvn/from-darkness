#pragma once

#include "fd/comptr.h"
#include "fd/core.h"

#include <Windows.h>
#include <d3d9.h>
#include <tchar.h>

namespace fd
{
struct d3d_device9
{
    using pointer = IDirect3DDevice9 *;
    using value_type=IDirect3DDevice9;

  private:
    comptr<IDirect3D9> d3d_;
    comptr<IDirect3DDevice9> device_;
    D3DPRESENT_PARAMETERS params_;

  public:
    d3d_device9();

    bool attach(HWND hwnd);
    bool resize(UINT w, UINT h);
    void reset();

    operator pointer() const;
    pointer get() const;
    pointer operator->() const;
};

struct own_render_backend : noncopyable
{
    using device_type = d3d_device9;

  private:
    WNDCLASSEX info_;
    HWND hwnd_;
    device_type device_;

  public:
    ~own_render_backend();
    own_render_backend(LPCTSTR name, HMODULE handle = GetModuleHandle(nullptr), HWND parent = nullptr);
    bool run();
    bool stop();

    device_type::pointer backend() const;
    HWND window() const;
    WNDPROC window_proc() const;
    WNDPROC default_window_proc() const;
};
} // namespace fd