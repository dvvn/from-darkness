
#include "cheat/hooks/base_includes.h"
#include "cheat/netvars/includes.h"
#include "cheat/players/player_includes.h"

#include <nstd/runtime_assert.h>
#include <nstd/winapi/comptr_includes.h>

#include <Windows.h>

#include <imgui.h>
#include <d3d9.h>
#include <tchar.h>

import cheat.console;
import cheat.csgo.interfaces;
import cheat.gui;
import cheat.hooks.winapi;
import cheat.hooks.imgui;
import cheat.hooks.directx;
import cheat.hooks.winapi;
import nstd.winapi;

//#define RUN_HOOKS_TEST

#ifdef RUN_HOOKS_TEST
#include <dhooks/includes.h>
import dhooks;

struct target_struct
{
	int __stdcall target_func( )
	{
		return 999;
	}
};

int __fastcall target_func( )
{
	return 888;
}

struct test_struct_hook : dhooks::select_hook_holder<decltype(&target_struct::target_func)>
{
	void callback( ) override
	{
		this->store_return_value(9991);
	}

	void* get_target_method( ) const override
	{
		return dhooks::pointer_to_class_method(&target_struct::target_func);
	}
};

struct test_fn_hook : dhooks::select_hook_holder<decltype(target_func)>
{
	void callback( ) override
	{
		this->store_return_value(8881);
	}

	void* get_target_method( ) const override
	{
		return target_func;
	}
};

template<class T>
static auto make_hook( )
{
	auto hook = std::make_unique<T>( );
	auto h = hook->hook( );
	runtime_assert(h == true);
	auto e = hook->enable( );
	runtime_assert(e == true);
	return hook;
}

static void run_hooks_test( )
{
	dhooks::current_context::set(std::make_shared<dhooks::context>( ));
	auto struct_hook = make_hook<test_struct_hook>( );
	auto fn_hook = make_hook<test_fn_hook>( );
	dhooks::current_context::reset( );

	target_struct obj;

	auto hooked_struct = obj.target_func( );
	struct_hook.reset( );
	auto original_struct = obj.target_func( );

	auto hooked_fn = target_func( );
	fn_hook->disable( );
	auto original_fn = target_func( );

	DebugBreak( );
}

#endif

using nstd::winapi::comptr;

// Data
static comptr<IDirect3D9> g_pD3D;
static comptr<IDirect3DDevice9> g_pd3dDevice;
static D3DPRESENT_PARAMETERS g_d3dpp;

// Forward declarations of helper functions
static bool CreateDeviceD3D(HWND hWnd);
static void ResetDevice( );
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void register_services( )
{
	using namespace cheat;
	auto deps = services_loader::get( ).deps( );
	deps.add<console>( );
	deps.add<csgo_interfaces>( )->prepare_for_gui_test(g_pd3dDevice);
	deps.add<gui::menu>( );
	using namespace hooks;
	deps.add<winapi::wndproc>( );
	deps.add<imgui::PushClipRect>( );
	deps.add<directx::reset>( );
	deps.add<directx::present>( );
}

// Main code
int main(int, char**)
{
#ifdef RUN_HOOKS_TEST 
	run_hooks_test( );
#endif

	using namespace cheat;

	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	const WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("ImGui Example"), nullptr};
	::RegisterClassEx(&wc);
	const HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	register_services( );

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	const bool loaded = services_loader::get( ).start( );
	if (!loaded)
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
	for (;;)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				goto _RESET; 
		}

		//unload called
		if (services_loader::get( ).state( ) == service_state::unset)
			goto _RESET;

		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
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

	services_loader::get( ).reset(false);

	/*ImGui_ImplDX9_Shutdown( );
	ImGui_ImplWin32_Shutdown( );
	ImGui::DestroyContext( );*/

	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	g_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if (!g_pD3D)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	return SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, g_pd3dDevice));
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
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
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
