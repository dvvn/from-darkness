#include "own_dx11.h"
//
#include "comptr.h"
#include "diagnostics/system_error.h"

#include <tchar.h>

#include <cassert>

namespace fd
{

own_dx11_backend_data::own_dx11_backend_data(HWND hwnd)
{
    auto const d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d)
        throw system_error("D3D device not created!");

    d3d_.Attach(d3d);
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

    auto const result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params_, device_);

    if (FAILED(result))
        throw system_error(result, "D3D device create error");
}

void own_dx11_backend::reset()
{
    basic_dx11_backend::reset();
    auto const hr = device_->Reset(&params_);
    assert(hr != D3DERR_INVALIDCALL);
}

own_dx11_backend::own_dx11_backend(HWND hwnd)
    : own_dx11_backend_data(hwnd)
    , basic_dx11_backend(device_)
{
}

void own_dx11_backend::render(ImDrawData* draw_data)
{
    device_->SetRenderState(D3DRS_ZENABLE, FALSE);
    device_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device_->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    auto const clear = device_->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    assert(clear == D3D_OK);

    auto const begin = device_->BeginScene();
    assert(begin == D3D_OK);

    basic_dx11_backend::render(draw_data);

    auto const end = device_->EndScene();
    assert(end == D3D_OK);

    auto const present = device_->Present(nullptr, nullptr, nullptr, nullptr);
    if (present == D3DERR_DEVICELOST && device_->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        own_dx11_backend::reset();
    else
        assert(present == D3D_OK);
}

void own_dx11_backend::resize(UINT w, UINT h)
{
#ifdef _DEBUG
    [[maybe_unused]] auto last_w = params_.BackBufferWidth;
    [[maybe_unused]] auto last_h = params_.BackBufferHeight;
#endif
    auto do_reset = false;
    if (auto& w_old = params_.BackBufferWidth; w_old != w)
    {
        w_old    = w;
        do_reset = true;
    }
    if (auto& h_old = params_.BackBufferHeight; h_old != h)
    {
        h_old    = h;
        do_reset = true;
    }
    if (do_reset)
        own_dx11_backend::reset();
}

void* own_dx11_backend::native() const
{
    return device_.Get();
}
} // namespace fd