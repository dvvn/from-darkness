#include "imgui context.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/core/services loader.h"

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
	HWND         hwnd = nullptr;
};

HWND imgui_context::hwnd( ) const
{
	return data_->hwnd;
}

// ReSharper disable once CppMemberFunctionMayBeConst
ImGuiContext& imgui_context::get( )
{
	return data_->ctx;
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
	data_ = std::make_unique<data_type>( );
	this->wait_for_service<csgo_interfaces>( );
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

	auto& io = data_->ctx.IO;

	const auto set_style = [&]
	{
		[[maybe_unused]]
			auto& style = data_->ctx.Style;
#if defined(IMGUI_HAS_SHADOWS) && IMGUI_HAS_SHADOWS == 1

		/*auto& shadow_cfg = io.Fonts->ShadowTexConfig;

		shadow_cfg.TexCornerSize          = 16 ;
		shadow_cfg.TexEdgeSize            = 1;
		shadow_cfg.TexFalloffPower        = 4.8f ;
		shadow_cfg.TexDistanceFieldOffset = 3.8f ;*/

		style.WindowShadowSize              = 30;
		style.WindowShadowOffsetDist        = 10;
		style.Colors[ImGuiCol_WindowShadow] = /*ImColor(0, 0, 0, 255)*/style.Colors[ImGuiCol_WindowBg];
#endif
	};

	set_style( );

	io.IniFilename = nullptr;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	io.FontDefault = [&]
	{
		ImFontConfig font_cfg;
		//font_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint;
		static ImWchar ranges[] =
		{
			0x0020, IM_UNICODE_CODEPOINT_MAX, //almost language of utf8 range
			0,
		};
		font_cfg.SizePixels  = 13;
		font_cfg.OversampleH = 2;
		font_cfg.OversampleV = 1;
		font_cfg.PixelSnapH  = true;
		font_cfg.GlyphRanges = /*io.Fonts->GetGlyphRangesCyrillic( )*/ranges;

#if !defined(_DEBUG) && 0
		return io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 15.0f, std::addressof(font_cfg), nullptr);
#else
		return io.Fonts->AddFontDefault(std::addressof(font_cfg));
#endif

		/*if(font->FontSize==0)
		{
			for (const auto& d: io.Fonts->ConfigData)
			{
				if (d.DstFont == font)
				{
					font->FontSize = d.SizePixels;
					break;
				}
			}
		}*/
	}( );

	auto creation_parameters = D3DDEVICE_CREATION_PARAMETERS( );

	[[maybe_unused]] const auto result = d3d->GetCreationParameters(&creation_parameters);
	runtime_assert(SUCCEEDED(result));

	data_->hwnd = creation_parameters.hFocusWindow;
	ImGui_ImplWin32_Init(data_->hwnd);
	ImGui_ImplDX9_Init(d3d);
	//ImGui_ImplDX9_CreateDeviceObjects( ); //d3d9 multithread error

	io.Fonts->Build();

	co_return service_state::loaded;
}

CHEAT_REGISTER_SERVICE(imgui_context);
