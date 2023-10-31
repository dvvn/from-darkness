#include "comptr.h"
#include "diagnostics/system_error.h"
#include "gui/render/backend/own_dx9.h"

#include <tchar.h>

#include <cassert>

namespace fd::gui
{
own_dx9_backend_data::own_dx9_backend_data(HWND hwnd)
{
    auto const d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d)
        throw system_error("D3D device not created!");
    d3d_ = (d3d);
#ifndef _DEBUG
    memset(&params_, 0, sizeof(D3DPRESENT_PARAMETERS));
#endif
#if 0
        RECT windowRect;
        // Get a handle to the desktop window
        // Get the size of screen to the variable desktop
        GetWindowRect(hwnd, &windowRect);
        // The top left corner will have coordinates (0,0)
        // and the bottom right corner will have coordinates
        UINT width  = windowRect.right - windowRect.left;
        UINT height = windowRect.bottom - windowRect.top;
#endif
    params_ = {
        //.BackBufferWidth        = width,
        //.BackBufferHeight       = height,
        .BackBufferFormat       = D3DFMT_UNKNOWN, // Need to use an explicit format with alpha if needing per-pixel alpha composition.
        .SwapEffect             = D3DSWAPEFFECT_DISCARD,
        //.hDeviceWindow          = hwnd,
        .Windowed               = TRUE,
        .EnableAutoDepthStencil = TRUE,
        .AutoDepthStencilFormat = D3DFMT_D16,
        .PresentationInterval   = D3DPRESENT_INTERVAL_ONE // vsync on
        //.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE // vsync off
    };

    auto const result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params_, &device_);

    if (FAILED(result))
        throw system_error(result, "D3D device create error");
}

void own_dx9_backend::reset()
{
    basic_dx9_backend::reset();
    auto const hr = device_->Reset(&params_);
    assert(hr != D3DERR_INVALIDCALL);
}

own_dx9_backend::own_dx9_backend(HWND hwnd)
    : own_dx9_backend_data(hwnd)
    , basic_dx9_backend(device_)
{
}

void own_dx9_backend::render(ImDrawData* draw_data)
{
    device_->SetRenderState(D3DRS_ZENABLE, FALSE);
    device_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device_->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    auto const clear = device_->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    assert(clear == D3D_OK);

    auto const begin = device_->BeginScene();
    assert(begin == D3D_OK);

    basic_dx9_backend::render(draw_data);

    auto const end = device_->EndScene();
    assert(end == D3D_OK);

    auto const present = device_->Present(nullptr, nullptr, nullptr, nullptr);
    if (present == D3DERR_DEVICELOST && device_->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        own_dx9_backend::reset();
    else
        assert(present == D3D_OK);
}

void own_dx9_backend::resize(simple_win32_window_size const& size)
{
#if INT_MAX == LONG_MAX
#ifdef _DEBUG
    [[maybe_unused]] //
    std::pair const last(params_.BackBufferWidth, params_.BackBufferHeight);
#endif
    auto const& current = reinterpret_cast<simple_win32_window_size&>(params_.BackBufferWidth);
#else
    simple_win32_window_size const current(params_.BackBufferWidth, params_.BackBufferHeight);
#endif

    if (current != size)
        own_dx9_backend::reset();
}
} // namespace fd::gui