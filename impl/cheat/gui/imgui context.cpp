// ReSharper disable CppMemberFunctionMayBeConst
#include "imgui context.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/core/services loader.h"

#include <imgui_internal.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

#include <excpt.h>
#include <filesystem>
#include <fstream>
#include <optional>

using namespace cheat;
using namespace gui;

struct fonts_builder_proxy::impl
{
	~impl( )
	{
		if (!atlas)
			return;
		if (atlas->ConfigData.size( ) == known_fonts)
			return;

		atlas->Build( );
	}

	ImFontAtlas* atlas       = 0;
	int          known_fonts = 0;
};

fonts_builder_proxy::fonts_builder_proxy( )
{
	impl_ = std::make_unique<impl>( );
}

fonts_builder_proxy::fonts_builder_proxy(ImFontAtlas* atlas)
	: fonts_builder_proxy( )
{
	impl_->atlas       = atlas;
	impl_->known_fonts = atlas->ConfigData.size( );
}

fonts_builder_proxy::~fonts_builder_proxy( )                                              = default;
fonts_builder_proxy::fonts_builder_proxy(fonts_builder_proxy&& other) noexcept            = default;
fonts_builder_proxy& fonts_builder_proxy::operator=(fonts_builder_proxy&& other) noexcept = default;

ImFont* fonts_builder_proxy::add_default_font(std::optional<ImFontConfig>&& cfg_opt)
{
	if (!cfg_opt.has_value( ))
		cfg_opt = default_font_config( );

	return impl_->atlas->AddFontDefault(std::addressof(*cfg_opt));
}

ImFont* fonts_builder_proxy::add_font_from_ttf_file(const std::filesystem::path& path, std::optional<ImFontConfig>&& cfg_opt)
{
	if (!cfg_opt.has_value( ))
		cfg_opt = default_font_config( );

	auto&      cfg   = *cfg_opt;
	const auto atlas = impl_->atlas;

	if (cfg.FontDataOwnedByAtlas)
	{
		const auto file = path.string( );
		return atlas->AddFontFromFileTTF(file.c_str( ), cfg.SizePixels, std::addressof(cfg));
	}
		// ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
	else
	{
		auto infile = std::ifstream(path, std::ios::in | std::ios::binary | std::ios::ate);

		//infile.seekg(0, infile.end); //done by std::ios::ate
		const auto size = infile.tellg( );
		infile.seekg(0, std::ios::beg);

		const auto buffer = std::make_unique<uint8_t[]>(size);
		infile.read(reinterpret_cast<char*>(buffer.get( )), size);

		runtime_assert(!infile.bad( ));

		if (cfg.Name[0] == '\0')
		{
			const auto font_size    = static_cast<uint32_t>(cfg.SizePixels);
			auto       font_size_ex = font_size == 0 ? "?" : std::to_string(font_size);

			auto font_info = std::format("{}, {}px", path.filename( ).string( ), font_size);
			runtime_assert(font_info.size( ) + 1 <= std::size(cfg.Name));
			std::ranges::copy(font_info, cfg.Name);
		}

		return this->add_font_from_memory_ttf_file({buffer.get( ), static_cast<size_t>(size)}, std::move(cfg_opt));
	}
}

ImFont* fonts_builder_proxy::add_font_from_memory_ttf_file(const std::span<uint8_t>& buffer, std::optional<ImFontConfig>&& cfg_opt)
{
	if (!cfg_opt.has_value( ))
		cfg_opt = default_font_config( );

	auto&      cfg   = *cfg_opt;
	const auto atlas = impl_->atlas;

	runtime_assert(cfg.FontData == nullptr);

	cfg.FontData     = buffer._Unchecked_begin( );
	cfg.FontDataSize = buffer.size( );

	return atlas->AddFont(std::addressof(cfg));
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
	font_cfg.PixelSnapH           = false;
	font_cfg.GlyphRanges          = /*io.Fonts->GetGlyphRangesCyrillic( )*/ranges;

	return font_cfg;
}

//-------------

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

ImGuiContext& imgui_context::get( )
{
	return data_->ctx;
}

fonts_builder_proxy imgui_context::fonts( ) const
{
	return {std::addressof(data_->fonts)};
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
	const auto d3d = csgo_interfaces::get_ptr( )->d3d_device.get( );

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

	//----------

	data_->hwnd = [&]
	{
		auto creation_parameters = D3DDEVICE_CREATION_PARAMETERS( );

		[[maybe_unused]] const auto result = d3d->GetCreationParameters(&creation_parameters);
		runtime_assert(SUCCEEDED(result));
		return creation_parameters.hFocusWindow;
	}( );
	ImGui_ImplWin32_Init(data_->hwnd);
	ImGui_ImplDX9_Init(d3d);

	//-----------

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

		style.WindowShadowSize = 30;
		style.WindowShadowOffsetDist = 10;
		style.Colors[ImGuiCol_WindowShadow] = /*ImColor(0, 0, 0, 255)*/style.Colors[ImGuiCol_WindowBg];
#endif
	};

	set_style( );

	io.IniFilename = nullptr;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	io.FontDefault = [&]
	{
		auto fonts = this->fonts( );

		auto font_cfg        = fonts.default_font_config( );
		font_cfg->SizePixels = 13;

#if /*!defined(_DEBUG)*/0
		return fonts.add_font_from_ttf_file("C:\\Windows\\Fonts\\arial.ttf", std::move(font_cfg));
#else
		return fonts.add_default_font(std::move(font_cfg));
#endif
	}( );

	co_return service_state::loaded;
}

CHEAT_REGISTER_SERVICE(imgui_context);
