#pragma once
#include "cheat/gui/tools/animator.h"
#include "cheat/gui/tools/string wrapper.h"

#define CHEAT_GUI_WINDOW_FADE_CONTENT

namespace cheat::gui::widgets
{
	class window
	{
	public:
		window(tools::animator&& fade = { });

		bool begin(const tools::string_wrapper& title, ImGuiWindowFlags_ flags);
		void end( );

		void show( );
		void hide( );
		void toggle( );

		bool visible( ) const;
		bool animating( ) const;
		bool active( ) const;

	private:
		bool ignore_end__ = false;
		bool visible__ = false;
		tools::animator fade__;
#ifdef CHEAT_GUI_WINDOW_FADE_CONTENT
		utl::memory_backup<float> fade_alpha_backup__;
#endif
	};
}
