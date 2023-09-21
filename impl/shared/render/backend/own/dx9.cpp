#include "comptr.h"
#include "dx9.h"
#include "name.h"
#include "noncopyable.h"
#include "object_holder.h"
#include "diagnostics/system_error.h"

#include <d3d9.h>
#include <tchar.h>

#include <cassert>

// #pragma comment(lib, "d3d9.lib")

namespace fd
{
struct dx9_holder : noncopyable
{
    using value_type = IDirect3DDevice9;
    using pointer    = value_type*;

  private:
    comptr<IDirect3D9> d3d_;
    comptr<IDirect3DDevice9> device_;
    D3DPRESENT_PARAMETERS params_;

  public:
    dx9_holder(HWND hwnd)
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

    bool resize(UINT w, UINT h)
    {
        auto ret = false;
        if (auto& w_old = params_.BackBufferWidth; w_old != w)
        {
            ret   = true;
            w_old = w;
        }
        if (auto& h_old = params_.BackBufferHeight; h_old != h)
        {
            h_old = h;
            ret   = true;
        }
        return ret;
    }

    void reset()
    {
        auto hr = device_->Reset(&params_);
        assert(hr != D3DERR_INVALIDCALL);
    }

    operator pointer() const
    {
        return device_;
    }

    auto get() const -> pointer
    {
        return device_;
    }

    auto operator->() const -> pointer
    {
        return device_;
    }
};

struct dx9_holder_proxy
{
    dx9_holder dx9;

    dx9_holder_proxy(HWND hwnd)
        : dx9(hwnd)
    {
    }
};

class own_dx9_backend final : dx9_holder_proxy, public basic_own_dx9_backend
{
    void reset() override
    {
        basic_dx9_backend::reset();
        dx9.reset();
    }

  public:
    ~own_dx9_backend() override
    {
        basic_dx9_backend::destroy();
    }

    own_dx9_backend()
        : dx9_holder_proxy(FindWindow(own_backend_class_name, nullptr))
        , basic_own_dx9_backend(dx9.get())
    {
    }

    void render(ImDrawData* draw_data) override
    {
        auto device = dx9.get();
        auto clear  = device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        assert(clear == D3D_OK);

        auto begin = device->BeginScene();
        assert(begin == D3D_OK);

        basic_dx9_backend::render(draw_data);

        auto end = device->EndScene();
        assert(end == D3D_OK);

        auto present = device->Present(nullptr, nullptr, nullptr, nullptr);
        if (present == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            own_dx9_backend::reset();
        else
            assert(present == D3D_OK);
    }

    void resize(UINT w, UINT h) override
    {
        if (dx9.resize(w, h))
            own_dx9_backend::reset();
    }

    void* native() const override
    {
        return dx9.get();
    }
};

basic_own_dx9_backend* make_incomplete_object<own_dx9_backend>::operator()() const
{
    return make_object<own_dx9_backend>();
}
} // namespace fd