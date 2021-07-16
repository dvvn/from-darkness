#include "user input.h"

#include "cheat/core/csgo interfaces.h"
//#include "cheat/hooks/input/wndproc.h"

#include "menu.h"

using namespace cheat;
using namespace gui;
using namespace hooks;
using namespace utl;

user_input::user_input( )
{
	this->Wait_for<csgo_interfaces>( );
	//this->Wait_for<menu_obj>( );
}

user_input::~user_input( )
{
	ImGui_ImplWin32_Shutdown( );
	if (ctx__ != nullptr)
		ImGui::DestroyContext(ctx__);
}

HWND user_input::hwnd( ) const
{
	return hwnd__;
}

void user_input::Load( )
{
	IMGUI_CHECKVERSION( );
	ImGui::SetAllocatorFunctions([](size_t size, void*) { return operator new(size); },
								 [](void*  ptr, void* ) { return operator delete(ptr); });
	ctx__ = ImGui::CreateContext( );
	auto& io = ctx__->IO;
	io.IniFilename = nullptr;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImFontConfig font_cfg;
	//font_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint;
	static ImWchar ranges[] =
	{
		0x0020, 0xFFFF, //almost language of utf8 range
		0,
	};
	font_cfg.OversampleH = 2;
	font_cfg.OversampleV = 1;
	//font_cfg.PixelSnapH = true;
	//font_cfg.EllipsisChar = (ImWchar)0x0085;
	font_cfg.GlyphOffset.y = 1.0f;
	font_cfg.GlyphRanges = /*io.Fonts->GetGlyphRangesCyrillic( )*/ranges;
#if defined(NDEBUG) && 0
	io.FontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arialuni.ttf", 15.0f, addressof(font_cfg), nullptr);
#else

	io.Fonts->AddFontDefault(addressof(font_cfg));

#endif
	auto creation_parameters = D3DDEVICE_CREATION_PARAMETERS( );

	const auto result = csgo_interfaces::get( ).d3d_device->GetCreationParameters(&creation_parameters);
	BOOST_ASSERT(SUCCEEDED(result));

	hwnd__ = creation_parameters.hFocusWindow;
	ImGui_ImplWin32_Init(hwnd__);
}

// ReSharper disable once CppInconsistentNaming
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

user_input::process_result user_input::process(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	(void)this;

	auto& menu = menu::get( );

	const auto skip_input = [&]
	{
		//todo: if skipped -> render last filled buffer
		switch (msg)
		{
			case WM_CLOSE:
			case WM_DESTROY:
			case WM_QUIT:
			case WM_SYSCOMMAND:
			case WM_MOVE:
			case WM_SIZE:
			case WM_KILLFOCUS:
			case WM_SETFOCUS:
			case WM_ACTIVATE:
				return process_result::none;
			default:
				return process_result::skipped;
		}
	};

	if (menu.toggle(msg, wparam) || menu.animating( ))
	{
		return skip_input( );
	}
	if (menu.active( ))
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
			return process_result::blocked;
		return skip_input( );
	}
	return process_result::none;
}
