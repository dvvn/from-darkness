#pragma once
#include "pages renderer.h"

#include "cheat/core/service.h"
#include "cheat/gui/animator.h"

namespace cheat::gui
{
	class menu_obj final: public service_shared<menu_obj, service_mode::async>
	{
	public:
		menu_obj( );

		void render(float bg_alpha);

		bool visible( ) const;
		bool animating( ) const;
		bool active( ) const;

		bool toggle(UINT msg, WPARAM wparam);
		void toggle( );
		bool animate( );

		float get_fade( ) const;

	protected:
		void Load( ) override;

	private:
		imgui::string_wrapper menu_title__;
		menu::vertical_pages_renderer renderer__;

		bool visible__ = false;
		animator fade__;
		WPARAM hotkey__ = VK_HOME;
	};
}
