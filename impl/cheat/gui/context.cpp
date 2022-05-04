module;

#include <nstd/runtime_assert.h>

#include <imgui_internal.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

#include <functional>
#include <filesystem>
#include <fstream>

module cheat.gui:context;
import cheat.console.object_message;

using namespace cheat;
using namespace gui;

ImFontConfig fonts_builder_proxy::default_font_config( )
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

ImFont* fonts_builder_proxy::add_default_font(const ImFontConfig & cfg)
{
	return atlas_->AddFontDefault(std::addressof(cfg));
}

template<size_t N, typename ...T>
static void append_to_buffer(char(&buffer)[N], const T ...names)
{
	size_t offset = 0;
	const auto copy_impl = [&](const std::string_view name)
	{
		std::copy(name.begin( ), name.end( ), buffer + offset);
		offset += name.size( );
		runtime_assert(offset <= N, "Buffer too small!");
	};

	(copy_impl(names), ...);
	buffer[offset] = '\0';
}

class memory_file_data
{
public:
	uint8_t* begin( )const
	{
		return buff_.get( );
	}
	uint8_t* end( )const
	{
		return buff_.get( ) + size_;
	}

	size_t size( )const
	{
		return size_;
	}

	memory_file_data(const std::filesystem::path& source)
	{
		using namespace std;
		auto infile = ifstream(source, ios::in | ios::binary | ios::ate);
		size_ = static_cast<size_t>(infile.tellg( ));

		infile.seekg(0, ios::beg);

		buff_ = make_unique<uint8_t[]>(size_);
		infile.read((char*)buff_.get( ), size_);
		runtime_assert(!infile.bad( ));
	}

private:
	std::unique_ptr<uint8_t[]>buff_;
	size_t size_;
};

ImFont* fonts_builder_proxy::add_font_from_ttf_file(const std::filesystem::path & path, ImFontConfig && cfg)
{
	if (cfg.FontDataOwnedByAtlas)
	{
		const auto file = path.string( );
		return atlas_->AddFontFromFileTTF(file.c_str( ), cfg.SizePixels, std::addressof(cfg));
	}

	if (cfg.Name[0] == '\0')
	{
		const auto font_size = static_cast<size_t>(cfg.SizePixels);
		runtime_assert(cfg.SizePixels - static_cast<float>(font_size) == 0);

		append_to_buffer(cfg.Name,
						 path.filename( ).string( ),
						 ", ",
						 font_size == 0 ? "?" : std::to_string(font_size),
						 "px");
	}

	const auto buffer = memory_file_data(path);
	return this->add_font_from_memory_ttf_file(buffer.begin( ), buffer.end( ), std::move(cfg));
}

ImFont* fonts_builder_proxy::add_font_from_memory_ttf_file(uint8_t * buffer_start, uint8_t * buffer_end, ImFontConfig && cfg)
{
	runtime_assert(buffer_start < buffer_end);
	runtime_assert(cfg.FontData == nullptr);
	cfg.FontData = buffer_start;
	cfg.FontDataSize = std::distance(buffer_start, buffer_end);

	return atlas_->AddFont(std::addressof(cfg));
}

//-------------

context::~context( )
{
	constexpr auto safe_call = []<typename T>(T fn)
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
	safe_call(ImGui::Shutdown);

	console::object_destroyed<context>( );
}

context::context( )
	: ImGuiContext(std::addressof(fonts))
{

	console::object_created<context>( );

	IMGUI_CHECKVERSION( );
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
	ImGui::SetAllocatorFunctions([](size_t size, void*)
	{
		return operator new(size);
	}, [](void* ptr, void*)
	{
		return operator delete(ptr);
	});
#endif

	ImGui::SetCurrentContext(this);
	ImGui::Initialize( );

	//----------

	hwnd = [&]
	{
		auto creation_parameters = D3DDEVICE_CREATION_PARAMETERS( );

		[[maybe_unused]] const auto result = nstd::instance_of<IDirect3DDevice9*>->GetCreationParameters(&creation_parameters);
		runtime_assert(SUCCEEDED(result));
		return creation_parameters.hFocusWindow;
	}();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(&nstd::instance_of<IDirect3DDevice9*>);

#if defined(IMGUI_HAS_SHADOWS) && IMGUI_HAS_SHADOWS == 1
	/*auto& shadow_cfg = io.Fonts->ShadowTexConfig;

	shadow_cfg.TexCornerSize          = 16 ;
	shadow_cfg.TexEdgeSize            = 1;
	shadow_cfg.TexFalloffPower        = 4.8f ;
	shadow_cfg.TexDistanceFieldOffset = 3.8f ;*/

	Style.WindowShadowSize = 30;
	Style.WindowShadowOffsetDist = 10;
	Style.Colors[ImGuiCol_WindowShadow] = /*ImColor(0, 0, 0, 255)*/Style.Colors[ImGuiCol_WindowBg];
#endif
	Style.Colors[ImGuiCol_WindowBg].w = 0.7f;
	Style.Colors[ImGuiCol_PopupBg].w = 0.5f;

	IO.IniFilename = nullptr;
	//IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	IO.FontDefault = [&]
	{
		auto fnt = this->fonts_builder( );

		auto font_cfg = fnt.default_font_config( );
		font_cfg.SizePixels = 13;

#if /*!defined(_DEBUG)*/0
		return fnt.add_font_from_ttf_file("C:\\Windows\\Fonts\\arial.ttf", std::move(font_cfg));
#else
		return fnt.add_default_font(font_cfg);
#endif
	}();
}

bool context::inactive( ) const
{
	return (hwnd != GetForegroundWindow( ));
}

fonts_builder_proxy context::fonts_builder( )
{
	return std::addressof(fonts);
}