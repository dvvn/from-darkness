#pragma once

#include "cheat/core/service.h"

#include "objects/pages renderer.h"
#include "widgets/window.h"

#if defined(_DEBUG) ||  defined(CHEAT_TEST_EXE)
#define CHEAT_GUI_HAS_DEMO_WINDOW 1
#else
#define CHEAT_GUI_HAS_DEMO_WINDOW 0
#endif

namespace cheat::gui
{
	class menu final: public service_shared<menu, service_mode::async>, public widgets::window
	{
		using window::begin;
		using window::end;

	public:
		menu( );

		void render( );
		bool toggle(UINT msg, WPARAM wparam);

	protected:
		void Load( ) override;

	private:
		tools::string_wrapper menu_title__;
		objects::vertical_pages_renderer renderer__;

		WPARAM hotkey__ = VK_HOME;
	};
}
