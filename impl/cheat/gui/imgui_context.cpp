module;

#include "cheat/service/includes.h"

#include <nstd/file/to_memory.h>
#include <nstd/format.h>

#include <imgui_internal.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

//#include <filesystem>
//#include <functional>
//#include <optional>
//#include <string>
//#include <variant>
//#include <sstream>
//#include <functional>
//#include <mutex>

module cheat.gui.context;
import cheat.csgo.interfaces;

using namespace cheat;
using namespace gui;

fonts_builder_proxy::fonts_builder_proxy(ImFontAtlas* atlas)
	: atlas_(atlas), known_fonts_(atlas->ConfigData.size( ))
{
}

fonts_builder_proxy::~fonts_builder_proxy( )
{
	if (!atlas_)
		return;
	if (atlas_->ConfigData.size( ) == known_fonts_)
		return;

	atlas_->Build( );
}
fonts_builder_proxy::fonts_builder_proxy(fonts_builder_proxy&& other) noexcept = default;
fonts_builder_proxy& fonts_builder_proxy::operator=(fonts_builder_proxy && other) noexcept = default;

ImFont* fonts_builder_proxy::add_default_font(std::optional<ImFontConfig> && cfg_opt)
{
	if (!cfg_opt.has_value( ))
		cfg_opt = default_font_config( );

	return atlas_->AddFontDefault(std::addressof(*cfg_opt));
}

ImFont* fonts_builder_proxy::add_font_from_ttf_file(const std::filesystem::path & path, std::optional<ImFontConfig> && cfg_opt)
{
	if (!cfg_opt.has_value( ))
		cfg_opt = default_font_config( );

	auto& cfg = *cfg_opt;
	ImFont* out;
	if (cfg.FontDataOwnedByAtlas)
	{
		const auto file = path.string( );
		out = atlas_->AddFontFromFileTTF(file.c_str( ), cfg.SizePixels, std::addressof(cfg));
	}
	else
	{
		namespace rn = std::ranges;
		const auto buffer = nstd::file::to_memory(path.native( ));

		if (cfg.Name[0] == '\0')
		{
			const auto font_size = static_cast<size_t>(cfg.SizePixels);
			runtime_assert(cfg.SizePixels - static_cast<float>(font_size) == 0);
			const auto font_size_ex = font_size == 0 ? "?" : std::to_string(font_size);

			auto font_info = std::format("{}, {}px", path.filename( ).string( ), font_size_ex);
			runtime_assert(font_info.size( ) + 1 <= rn::size(cfg.Name));
			rn::copy(font_info, cfg.Name);
			cfg.Name[font_info.size( )] = '\0';
		}

		out = this->add_font_from_memory_ttf_file(buffer.begin( ), buffer.end( ), std::move(cfg_opt));
	}

	return out;
}

ImFont* fonts_builder_proxy::add_font_from_memory_ttf_file(uint8_t * buffer_start, uint8_t * buffer_end, std::optional<ImFontConfig> && cfg_opt)
{
	runtime_assert(buffer_start < buffer_end);
	if (!cfg_opt.has_value( ))
		cfg_opt = default_font_config( );

	auto& cfg = *cfg_opt;
	runtime_assert(cfg.FontData == nullptr);
	cfg.FontData = buffer_start;
	cfg.FontDataSize = std::distance(buffer_start, buffer_end);

	return atlas_->AddFont(std::addressof(cfg));
}

std::optional<ImFontConfig> fonts_builder_proxy::default_font_config( )
{
	ImFontConfig font_cfg;
	//font_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint;
	static ImWchar ranges[] = {
		0x0020, IM_UNICODE_CODEPOINT_MAX, //almost language of utf8 range
		0
	};
	//font_cfg.OversampleH = 3;
	//font_cfg.OversampleV = 1;
	font_cfg.FontDataOwnedByAtlas = false;
	font_cfg.PixelSnapH = false;
	font_cfg.GlyphRanges = /*io.Fonts->GetGlyphRangesCyrillic( )*/ranges;

	return font_cfg;
}

//-------------

HWND context::hwnd( ) const
{
	return hwnd_;
}

ImGuiContext& context::get_context( )
{
	return ctx_;
}

fonts_builder_proxy context::fonts( )
{
	return std::addressof(fonts_);
}

context::~context( )
{
	constexpr auto safe_call = []<typename T>(T && fn)
	{
		__try
		{
			std::invoke(fn);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	};

	safe_call(ImGui_ImplWin32_Shutdown);
	safe_call(ImGui_ImplDX9_Shutdown);
	safe_call(std::bind_front(ImGui::Shutdown, std::addressof(ctx_)));
}

context::context( )
	: ctx_(std::addressof(fonts_))
{
}

bool context::inctive( ) const
{
	return (hwnd_ != GetForegroundWindow( ));
}

bool context::load_impl( ) noexcept
{
	const auto d3d = csgo_interfaces::get( )->d3d_device.get( );

	IMGUI_CHECKVERSION( );
	ImGui::SetAllocatorFunctions([](size_t size, void*)
								 {
									 return operator new(size);
								 },
								 [](void* ptr, void*)
								 {
									 return operator delete(ptr);
								 });

	ImGui::SetCurrentContext(std::addressof(ctx_));
	ImGui::Initialize(std::addressof(ctx_));

	//----------

	hwnd_ = [&]
	{
		auto creation_parameters = D3DDEVICE_CREATION_PARAMETERS( );

		[[maybe_unused]] const auto result = d3d->GetCreationParameters(&creation_parameters);
		runtime_assert(SUCCEEDED(result));
		return creation_parameters.hFocusWindow;
	}();
	ImGui_ImplWin32_Init(hwnd_);
	ImGui_ImplDX9_Init(d3d);

	//-----------

	auto& io = ctx_.IO;

	const auto set_style = [&]
	{
		auto& style = ctx_.Style;
#if defined(IMGUI_HAS_SHADOWS) && IMGUI_HAS_SHADOWS == 1

		/*auto& shadow_cfg = io.Fonts->ShadowTexConfig;

		shadow_cfg.TexCornerSize          = 16 ;
		shadow_cfg.TexEdgeSize            = 1;
		shadow_cfg.TexFalloffPower        = 4.8f ;
		shadow_cfg.TexDistanceFieldOffset = 3.8f ;*/

		style.WindowShadowSize = 30;
		style.WindowShadowOffsetDist = 10;
		style.Colors[ImGuiCol_WindowShadow] = /*ImColor(0, 0, 0, 255)*/style.Colors[ImGuiCol_WindowBg];
#endif

		style.Colors[ImGuiCol_WindowBg].w = 0.7f;
		style.Colors[ImGuiCol_PopupBg].w = 0.5f;
	};

	set_style( );

	io.IniFilename = nullptr;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	io.FontDefault = [&]
	{
		auto fnt = this->fonts( );

		auto font_cfg = fnt.default_font_config( );
		font_cfg->SizePixels = 13;

#if /*!defined(_DEBUG)*/0
		return fnt.add_font_from_ttf_file("C:\\Windows\\Fonts\\arial.ttf", std::move(font_cfg));
#else
		return fnt.add_default_font(std::move(font_cfg));
#endif
	}();


	return true;
}

CHEAT_SERVICE_REGISTER(context);
