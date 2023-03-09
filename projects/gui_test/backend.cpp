#include "backend.h"

#include <d3d9.h>
#include <tchar.h>

#include <cassert>

#define RESET_BACK_BUFFER_ON_RESIZE

namespace fd
{
static DECLSPEC_NOINLINE LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) // NOLINT(hicpp-multiway-paths-covered)
    {
    case WM_CREATE: {
        auto lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
        auto d3d  = reinterpret_cast<LONG>(lpcs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, d3d);
        break;
    }
    case WM_SIZE: {
        if (wparam == SIZE_MINIMIZED)
            break;

        auto &d3d = *reinterpret_cast<d3d_device9 *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (!d3d)
            break;

#ifdef RESET_BACK_BUFFER_ON_RESIZE
        if (!d3d.resize(LOWORD(lparam), HIWORD(lparam)))
            break;
        d3d.reset();
#else
#error "not implemented"
#endif
        return FALSE;
    }
    case WM_SYSCOMMAND: {
        if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return FALSE;
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return FALSE;
    }
    }
    return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

d3d_device9::d3d_device9() = default;

bool d3d_device9::attach(HWND hwnd)
{
    auto d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d)
        return false;

    d3d_.Attach(d3d);
    memset(&params_, 0, sizeof(D3DPRESENT_PARAMETERS));

#ifndef RESET_BACK_BUFFER_ON_RESIZE
    RECT windowRect;
    // Get a handle to the desktop window
    // Get the size of screen to the variable desktop
    GetWindowRect(hWnd, &windowRect);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    UINT width               = windowRect.right - windowRect.left;
    UINT height              = windowRect.bottom - windowRect.top;
    params_.BackBufferHeight = height;
    params_.BackBufferWidth  = width;
#endif
    params_.Windowed               = TRUE;
    params_.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    params_.BackBufferFormat       = D3DFMT_UNKNOWN;
    params_.EnableAutoDepthStencil = TRUE;
    params_.AutoDepthStencilFormat = D3DFMT_D16;
    // Present with vsync
    params_.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
    // Present without vsync, maximum unthrottled framerate
    // params_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    return SUCCEEDED(d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params_, device_));
}

bool d3d_device9::resize(UINT w, UINT h)
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

void d3d_device9::reset()
{
    auto hr = device_->Reset(&params_);
    assert(hr != D3DERR_INVALIDCALL);
}

d3d_device9::operator IDirect3DDevice9 *() const
{
    return device_;
}

IDirect3DDevice9 *d3d_device9::get() const
{
    return device_;
}

IDirect3DDevice9 *d3d_device9::operator->() const
{
    return device_;
}

backend_data::~backend_data()
{
    ::UnregisterClass(info.lpszClassName, info.hInstance);
    DestroyWindow(hwnd);
}

backend_data::backend_data()
{
    constexpr auto name = _T("GUI TEST");
    auto handle         = GetModuleHandle(nullptr);
    info                = {
        sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, handle, nullptr, nullptr, nullptr, nullptr, name, nullptr
    };
    ::RegisterClassEx(&info);
    hwnd = ::CreateWindow(name, name, WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, handle, &d3d);

    if (!d3d.attach(hwnd))
        return;

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
}

bool backend_data::run()
{
    assert(d3d != nullptr);

    for (;;)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                return true;
        }

        d3d->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        auto result = d3d->Present(nullptr, nullptr, nullptr, nullptr);
        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && d3d->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            d3d.reset();
    }
}
} // namespace fd