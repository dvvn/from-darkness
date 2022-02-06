module;

#include <imgui_internal.h>

#include <windows.h>

#include <filesystem>
#include <optional>

export module cheat.gui:context;
export import cheat.service;

export namespace cheat::gui
{
	class fonts_builder_proxy
	{
	public:
		fonts_builder_proxy(ImFontAtlas* atlas);
		~fonts_builder_proxy( );

		fonts_builder_proxy(const fonts_builder_proxy& other) = delete;
		fonts_builder_proxy& operator=(const fonts_builder_proxy& other) = delete;

		fonts_builder_proxy(fonts_builder_proxy&& other) noexcept;
		fonts_builder_proxy& operator=(fonts_builder_proxy&& other) noexcept;

		//todo: images loader

		ImFont* add_default_font(std::optional<ImFontConfig>&& cfg_opt);
		ImFont* add_font_from_ttf_file(const std::filesystem::path& path, std::optional<ImFontConfig>&& cfg_opt);
		ImFont* add_font_from_memory_ttf_file(uint8_t* buffer_start, uint8_t* buffer_end, std::optional<ImFontConfig>&& cfg_opt);

		static std::optional<ImFontConfig> default_font_config( );

	private:
		ImFontAtlas* atlas_ = nullptr;
		int known_fonts_ = 0;
	};

	struct context final : dynamic_service<context>
	{
		~context( ) override;
		context( );

		//todo: set in wndproc
		bool inactive( )const;
		HWND hwnd( ) const;
		ImGuiContext& access( );

		fonts_builder_proxy fonts( );

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;

	private:
		ImGuiContext ctx_;
		ImFontAtlas fonts_;
		//todo: move outside
		HWND hwnd_ = nullptr;
	};
}
