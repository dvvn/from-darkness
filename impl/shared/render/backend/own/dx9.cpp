#include "dx9.h"
//
#include "diagnostics/system_error.h"

#include <tchar.h>

#include <cassert>

// #pragma comment(lib, "d3d9.lib")

namespace fd
{
own_d3d_device::own_d3d_device(HWND hwnd)
{
    auto d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d)
        throw system_error("D3D device init error");

    d3d_.Attach(d3d);
#if 0
    RECT windowRect;
    // Get a handle to the desktop window
    // Get the size of screen to the variable desktop
    GetWindowRect(hwnd, &windowRect);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    UINT width               = windowRect.right - windowRect.left;
    UINT height              = windowRect.bottom - windowRect.top;
    params_.BackBufferHeight = height;
    params_.BackBufferWidth  = width;
#endif
    params_ = {
        // Need to use an explicit format with alpha if needing per-pixel alpha composition.
        .BackBufferFormat       = D3DFMT_UNKNOWN,
        .SwapEffect             = D3DSWAPEFFECT_DISCARD,
        .Windowed               = TRUE,
        .EnableAutoDepthStencil = TRUE,
        .AutoDepthStencilFormat = D3DFMT_D16,
        .PresentationInterval   = D3DPRESENT_INTERVAL_ONE // vsync on
        //.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE // vsync off
    };

    auto result = d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params_, device_);

    if (FAILED(result))
        throw system_error(result, "D3D device create error");
}

bool own_d3d_device::resize_device(UINT w, UINT h)
{
    auto ret = false;
    if (auto &w_old = params_.BackBufferWidth; w_old != w)
    {
        ret   = true;
        w_old = w;
    }
    if (auto &h_old = params_.BackBufferHeight; h_old != h)
    {
        h_old = h;
        ret   = true;
    }
    return ret;
}

void own_d3d_device::reset()
{
    auto hr = device_->Reset(&params_);
    assert(hr != D3DERR_INVALIDCALL);
}

own_d3d_device::operator pointer() const
{
    return device_;
}

auto own_d3d_device::get() const -> pointer
{
    return device_;
}

auto own_d3d_device::operator->() const -> pointer
{
    return device_;
}

//-------

dx9_backend_own::dx9_backend_own()
    : own_d3d_device(FindWindow(_T("__backend_win32"), nullptr))
    , basic_dx9_backend(own_d3d_device::get())
{
}

void dx9_backend_own::render(ImDrawData *draw_data)
{
    auto device = own_d3d_device::get();
    (void)device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    (void)device->BeginScene();
    basic_dx9_backend::render(draw_data);
    (void)device->EndScene();

    auto result = device->Present(nullptr, nullptr, nullptr, nullptr);
    // Handle loss of D3D9 device
    if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        dx9_backend_own::reset();
}

void dx9_backend_own::resize(UINT w, UINT h)
{
    if (resize_device(w, h))
        dx9_backend_own::reset();
}

IDirect3DDevice9 *dx9_backend_own::get() const
{
    return own_d3d_device::get();
}

void dx9_backend_own::reset()
{
    basic_dx9_backend::reset();
    own_d3d_device::reset();
}

dx9_backend_own::~dx9_backend_own()
{
    basic_dx9_backend::destroy();
}
} // namespace fd