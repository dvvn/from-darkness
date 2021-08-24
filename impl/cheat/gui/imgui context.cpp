#include "imgui context.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"
#include "cheat/hooks/vgui surface/lock cursor.h"

using namespace cheat;
using namespace hooks;
using namespace gui;

HWND imgui_context::hwnd( ) const
{
	return hwnd__;
}

imgui_context::~imgui_context( )
{
	std::invoke([]
	{
		__try
		{
			ImGui_ImplWin32_Shutdown( );
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
		__try
		{
			ImGui_ImplDX9_Shutdown( );
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	});

	ImGui::Shutdown(std::addressof(ctx__));
}

imgui_context::imgui_context( ): ctx__(&fonts__)
{
}

bool imgui_context::load_impl( )
{
	const auto interfaces = csgo_interfaces::get_ptr( );
	const auto d3d = interfaces->d3d_device.get( );

	IMGUI_CHECKVERSION( );
	ImGui::SetAllocatorFunctions([](size_t size, void*)
								 {
									 return operator new(size);
								 },
								 [](void* ptr, void*)
								 {
									 return operator delete(ptr);
								 });

	ImGui::SetCurrentContext(std::addressof(ctx__));
	ImGui::Initialize(std::addressof(ctx__));

	auto& io = ctx__.IO;
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
	font_cfg.PixelSnapH = true;
	font_cfg.GlyphRanges = /*io.Fonts->GetGlyphRangesCyrillic( )*/ranges;
#if !defined(_DEBUG) && 0
	io.FontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arialuni.ttf", 15.0f, addressof(font_cfg), nullptr);
#else

	io.Fonts->AddFontDefault(std::addressof(font_cfg));

#endif
	auto creation_parameters = D3DDEVICE_CREATION_PARAMETERS( );

	[[maybe_unused]] const auto result = d3d->GetCreationParameters(&creation_parameters);
	runtime_assert(SUCCEEDED(result));

	hwnd__ = creation_parameters.hFocusWindow;
	ImGui_ImplWin32_Init(hwnd__);
	ImGui_ImplDX9_Init(d3d);
	//ImGui_ImplDX9_CreateDeviceObjects( ); d3d9 multithread error

	return true;
}
