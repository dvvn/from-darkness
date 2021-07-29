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
		bool Do_load( ) override;

	private:
		HWND hwnd__ = nullptr;
		ImGuiContext ctx__;
		ImFontAtlas fonts__;
	};
}
