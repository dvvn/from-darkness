#pragma once
#include "cheat/core/service.h"

namespace cheat::gui
{
	class imgui_context final: public service<imgui_context>
	{
	public:
		~imgui_context( ) override;
		imgui_context( );

		HWND hwnd( ) const;

	protected:
		load_result load_impl( ) override;

	private:
		HWND         hwnd__ = nullptr;
		ImGuiContext ctx__;
		ImFontAtlas  fonts__;
	};
}
