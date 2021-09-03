#include "imgui context.h"

#include "cheat/core/csgo interfaces.h"

#include <imgui_internal.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

#include <excpt.h>

using namespace cheat;
using namespace gui;

struct imgui_context::data_type
{
	data_type( )
		: ctx(std::addressof(fonts))
	{
	}

	~data_type( )
	{
		ImGui::Shutdown(std::addressof(ctx));
	}

	data_type(const data_type&)            = delete;
	data_type& operator=(const data_type&) = delete;

	data_type(data_type&& other)            = delete;
	data_type& operator=(data_type&& other) = delete;

	ImGuiContext ctx;
	ImFontAtlas  fonts;
};

HWND imgui_context::hwnd( ) const
{
	return hwnd_;
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
}

imgui_context::imgui_context( )
{
	data_=std::make_unique<data_type>();
}

service_base::load_result imgui_context::load_impl( )
{
	const auto interfaces = csgo_interfaces::get_ptr( );
	const auto d3d        = interfaces->d3d_device.get( );

	IMGUI_CHECKVERSION( );
	ImGui::SetAllocatorFunctions([](size_t size, void*)
								 {
									 return operator new(size);
								 },
								 [](void* ptr, void*)
								 {
									 return operator delete(ptr);
								 });

	ImGui::SetCurrentContext(std::addressof(data_->ctx));
	ImGui::Initialize(std::addressof(data_->ctx));

	auto& io       = data_->ctx.IO;
	io.IniFilename = nullptr;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImFontConfig font_cfg;
	//font_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint;
	static ImWchar ranges[] =
	{
		0x0020, IM_UNICODE_CODEPOINT_MAX, //almost language of utf8 range
		0,
	};
	font_cfg.OversampleH = 2;
	font_cfg.OversampleV = 1;
	font_cfg.PixelSnapH  = true;
	font_cfg.GlyphRanges = /*io.Fonts->GetGlyphRangesCyrillic( )*/ranges;
#if !defined(_DEBUG) && 0
	io.FontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arialuni.ttf", 15.0f, addressof(font_cfg), nullptr);
#else

	io.Fonts->AddFontDefault(std::addressof(font_cfg));

#endif
	auto creation_parameters = D3DDEVICE_CREATION_PARAMETERS( );

	[[maybe_unused]] const auto result = d3d->GetCreationParameters(&creation_parameters);
	runtime_assert(SUCCEEDED(result));

	hwnd_ = creation_parameters.hFocusWindow;
	ImGui_ImplWin32_Init(hwnd_);
	ImGui_ImplDX9_Init(d3d);
	//ImGui_ImplDX9_CreateDeviceObjects( ); d3d9 multithread error

	co_return service_state::loaded;
}
