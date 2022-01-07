
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/sync_wait.hpp>

#include <Windows.h>

import cheat.service;
import cheat.gui.menu;

#pragma comment(lib, "Synchronization.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#ifndef CHEAT_GUI_TEST

// ReSharper disable once CppInconsistentNaming
extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	using namespace cheat;

	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		services_loader::get_ptr( )->load(hModule);
		break;
	case DLL_PROCESS_DETACH:
		services_loader::get_ptr( )->unload( );
		break;
	}

	return TRUE;
}

#else

#include <imgui.h>
#include <d3d9.h>
#include <tchar.h>

#pragma comment (lib, "d3d9.lib")

// Data
static LPDIRECT3D9 g_pD3D            = nullptr;
LPDIRECT3DDEVICE9 g_pd3dDevice       = nullptr;
static D3DPRESENT_PARAMETERS g_d3dpp = {};

// Forward declarations of helper functions
static bool CreateDeviceD3D(HWND hWnd);
static void CleanupDeviceD3D( );
static void ResetDevice( );
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	const WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("ImGui Example"), nullptr};
	::RegisterClassEx(&wc);
	const HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D( );
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	if (!cheat::services_loader::get( ).load( ))
		goto _RESET;

	// Setup Dear ImGui context
	//IMGUI_CHECKVERSION( );
	//ImGui::CreateContext( );
	//ImGuiIO& io = ImGui::GetIO( );
	//(void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark( );
	//ImGui::StyleColorsClassic();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state
	//bool   show_demo_window = true;
	//bool   show_another_window = false;

	// Main loop	
	while (1)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		MSG msg;
		bool done = false;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

		static ImVec4 clear_color    = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		static D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f),
													 (int)(clear_color.y * clear_color.w * 255.0f),
													 (int)(clear_color.z * clear_color.w * 255.0f),
													 (int)(clear_color.w * 255.0f));
		g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		const HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

		// Handle loss of D3D9 device
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET)
			ResetDevice( );
	}

_RESET:

	cheat::services_loader::get( ).unload( );

	/*ImGui_ImplDX9_Shutdown( );
	ImGui_ImplWin32_Shutdown( );
	ImGui::DestroyContext( );*/

	CleanupDeviceD3D( );
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed               = TRUE;
	g_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat       = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE; // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	return SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice));
}

void CleanupDeviceD3D( )
{
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release( );
		g_pd3dDevice = nullptr;
	}
	if (g_pD3D)
	{
		g_pD3D->Release( );
		g_pD3D = nullptr;
	}
}

void ResetDevice( )
{
	//ImGui_ImplDX9_InvalidateDeviceObjects( );
	const HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	//ImGui_ImplDX9_CreateDeviceObjects( );
}

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:
			if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
			{
				g_d3dpp.BackBufferWidth  = LOWORD(lParam);
				g_d3dpp.BackBufferHeight = HIWORD(lParam);
				ResetDevice( );
			}
			return 0;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

#endif
