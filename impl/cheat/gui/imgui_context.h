#pragma once

#include "cheat/service/include.h"

namespace std
{
	namespace filesystem
	{
		class path;
	}

	template <class Ty>
	class optional;
}

// ReSharper disable CppInconsistentNaming
struct ImGuiContext;
struct ImFont;
struct ImFontAtlas;
struct ImFontConfig;

struct HWND__;
using HWND = HWND__*;
// ReSharper restore CppInconsistentNaming

namespace cheat::gui
{
	class fonts_builder_proxy
	{
	public:
		fonts_builder_proxy( );
		fonts_builder_proxy(ImFontAtlas* atlas);
		~fonts_builder_proxy( );

		fonts_builder_proxy(const fonts_builder_proxy& other)            = delete;
		fonts_builder_proxy& operator=(const fonts_builder_proxy& other) = delete;

		fonts_builder_proxy(fonts_builder_proxy&& other) noexcept;
		fonts_builder_proxy& operator=(fonts_builder_proxy&& other) noexcept;

		//todo: images loader

		ImFont* add_default_font(std::optional<ImFontConfig>&& cfg_opt);
		ImFont* add_font_from_ttf_file(const std::filesystem::path& path, std::optional<ImFontConfig>&& cfg_opt);
		ImFont* add_font_from_memory_ttf_file( uint8_t* buffer_start,  uint8_t* buffer_end, std::optional<ImFontConfig>&& cfg_opt);

		static std::optional<ImFontConfig> default_font_config( );

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	class imgui_context_impl final : public service<imgui_context_impl>
	{
	public:
		~imgui_context_impl( ) override;
		imgui_context_impl( );

		bool inctive()const;
		HWND hwnd( ) const;
		ImGuiContext& get( );

		fonts_builder_proxy fonts( ) const;

	protected:
		load_result load_impl( ) noexcept override;

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};

	CHEAT_SERVICE_SHARE(imgui_context);
}