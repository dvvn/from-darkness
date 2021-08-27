#pragma once

#include "cheat/core/service.h"

#include "objects/pages renderer.h"
#include "widgets/window.h"

#if defined(_DEBUG) ||  defined(CHEAT_GUI_TEST)
#define CHEAT_GUI_HAS_DEMO_WINDOW 1
#else
#define CHEAT_GUI_HAS_DEMO_WINDOW 0
#endif

namespace cheat::gui
{
	class menu final: public service<menu>, public widgets::window
	{
		using window::begin;
		using window::end;

	public:
		menu( );

		void render( );
		bool toggle(UINT msg, WPARAM wparam);

	protected:
		load_result load_impl( ) override;

	private:
		tools::string_wrapper            menu_title_;
		objects::vertical_pages_renderer renderer_;

		WPARAM hotkey_ = VK_HOME;
	};
}
