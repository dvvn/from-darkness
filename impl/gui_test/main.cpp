//#include <nstd/runtime_assert.h>
#include <fd/comptr.h>

#include <d3d9.h>
#include <tchar.h>
#include <windows.h>

#include <compare>

import fd.hooks_loader;
import fd.logger;
import fd.logger.system_console;
import fd.application_info;
import fd.d3d_device9;
import fd.assert;

using nstd::winapi::comptr;

static comptr<IDirect3D9> g_pD3D;
static comptr<IDirect3DDevice9> g_pd3dDevice;
static D3DPRESENT_PARAMETERS g_d3dpp;

#define RESET_BACK_BUFFER_ON_RESIZE

static bool CreateDeviceD3D(HWND hWnd) noexcept
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
    const int width = desktop.right - desktop.left;
    const int height = desktop.bottom - desktop.top;
#endif

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
#ifndef RESET_BACK_BUFFER_ON_RESIZE
    g_d3dpp.BackBufferHeight = height;
    g_d3dpp.BackBufferWidth = width;
#endif
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
    // g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    return SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, g_pd3dDevice));
}

#if 0
static void PresetD3D(HWND nativeWindow)
{
	RECT clientRect;
	if(!GetClientRect(nativeWindow, &clientRect))
	{
		// if we can't lookup the client rect, abort, something is seriously wrong
		std::terminate( );
	}

	// Set up an orthographic projection.
	auto projection = DirectX::XMMatrixOrthographicOffCenterLH(0, (FLOAT)clientRect.right + 0.5f, (FLOAT)clientRect.bottom + 0.5f, 0, -1, 1);
	auto identity = DirectX::XMMatrixIdentity( );
	g_pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&identity);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&identity);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&projection);


	/*{
		float L = clientRect.left + 0.5f;
		float R = clientRect.left + clientRect.right + 0.5f;
		float T = clientRect.top + 0.5f;
		float B = clientRect.bottom + clientRect.top + 0.5f;
		D3DMATRIX mat_identity = {{ { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } }};
		D3DMATRIX mat_projection =
		{{ {
			2.0f / (R - L),   0.0f,         0.0f,  0.0f,
			0.0f,         2.0f / (T - B),   0.0f,  0.0f,
			0.0f,         0.0f,         0.5f,  0.0f,
			(L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f
		} }};
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
		g_pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
		g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
	}*/

	//-----

	// Switch to clockwise culling instead of counter-clockwise culling; Rocket generates counter-clockwise geometry,
	// so you can either reverse the culling mode when Rocket is rendering, or reverse the indices in the render
	// interface.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// Enable alpha-blending for Rocket.
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// Set up the texture stage states for the diffuse texture.
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Disable lighting for Rocket.
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}
#endif

static void ResetDevice() noexcept
{
    const HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    // runtime_assert(hr != D3DERR_INVALIDCALL);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
#ifdef RESET_BACK_BUFFER_ON_RESIZE
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
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

int main(int, char**)
{
    const WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("GUI TEST"), nullptr};
    ::RegisterClassEx(&wc);
    const HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("GUI TEST"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hwnd))
    {
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return TRUE;
    }

    // d3dDevice9_ptr = g_pd3dDevice;

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    using namespace fd;
    d3d_device9.construct(g_pd3dDevice);
    app_info.construct(hwnd, IsWindowUnicode(hwnd) ? GetModuleHandleW(nullptr) : GetModuleHandleA(nullptr));
    logger.append(system_console_writer);
    // PresetD3D(hwnd);
    if (!hooks_loader->load<0, 1, 2>())
        return TRUE;

    //----------------

    bool exit_application = false;
    while (!exit_application)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                goto _STOP;
        }

        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        const HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

_STOP:
    hooks_loader->disable();
    return FALSE;
}
