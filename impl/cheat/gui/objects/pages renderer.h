#pragma once
#include "abstract page.h"

#include "cheat/gui/widgets/group.h"
#include "cheat/gui/widgets/window.h"

#define CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING

namespace cheat::gui::objects
{
	class vertical_pages_renderer final: public abstract_pages_renderer, widgets::child_frame_window
	{
	public:
		void render( ) override;
		void init( ) override;

		using child_frame_window::show;

	private:
		size_t longest_string__ = 0;
		widgets::group selected_group__;
	};

	class horizontal_pages_renderer final: public abstract_pages_renderer, widgets::child_frame_window
	{
	public:

		horizontal_pages_renderer(/*size_t per_line_limit=-1*/)=default;

		void render( ) override;
		void init( ) override;

		using child_frame_window::show;

	private:
#ifdef CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING
		size_t longest_string__
#else
		size_t chars_count__
#endif
				= 0;
		widgets::group selected_group__;
		//size_t per_line_limit__; todo
	};
}
