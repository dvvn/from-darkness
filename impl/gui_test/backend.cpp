#include "backend.h"

#include <fd/assert.h>
#include <fd/comptr.h>

#include <d3d9.h>
#include <tchar.h>

// for readable assert
#ifdef _DEBUG
constexpr DWORD _D3DERR_INVALIDCALL = D3DERR_INVALIDCALL;
#undef D3DERR_INVALIDCALL
constexpr auto D3DERR_INVALIDCALL = _D3DERR_INVALIDCALL;
#endif

using namespace fd;

static comptr<IDirect3D9> g_pD3D;
static comptr<IDirect3DDevice9> g_pd3dDevice;
static D3DPRESENT_PARAMETERS g_d3dpp;

#define RESET_BACK_BUFFER_ON_RESIZE

static bool CreateDeviceD3D(HWND hWnd)
{
    g_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
    if (!g_pD3D)
        return false;

#ifndef RESET_BACK_BUFFER_ON_RESIZE
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    const int width  = desktop.right - desktop.left;
    const int height = desktop.bottom - desktop.top;
#endif

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
#ifndef RESET_BACK_BUFFER_ON_RESIZE
    g_d3dpp.BackBufferHeight = height;
    g_d3dpp.BackBufferWidth  = width;
#endif
    g_d3dpp.Windowed               = TRUE;
    g_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat       = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE; // Present with vsync
    // g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    return SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, g_pd3dDevice));
}

static void ResetDevice()
{
    const HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    FD_ASSERT(hr != D3DERR_INVALIDCALL);
}

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
#ifdef RESET_BACK_BUFFER_ON_RESIZE
            g_d3dpp.BackBufferWidth  = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
#else
#error "not implemented"
#endif
        }
        return FALSE;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return FALSE;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return FALSE;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

backend_data::~backend_data()
{
    ::UnregisterClass(name, handle);
    ::DestroyWindow(hwnd);
}

backend_data::backend_data()
{
    name                = _T("GUI TEST");
    handle              = GetModuleHandle(nullptr);
    const WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, handle, nullptr, nullptr, nullptr, nullptr, name, nullptr };
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(name, name, WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hwnd))
        return;

    d3d = g_pd3dDevice;

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
}

void backend_data::run()
{
    FD_ASSERT(d3d != nullptr);

    for (;;)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                return ;
        }

        d3d->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        const HRESULT result = d3d->Present(NULL, NULL, NULL, NULL);
        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && d3d->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }
}
