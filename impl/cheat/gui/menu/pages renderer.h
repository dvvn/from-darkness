#pragma once
#include "menu page.h"

namespace cheat::gui::menu
{
	class pages_renderer final: public abstract_pages_renderer
	{
	public:
		void render( ) override;
		void init( ) override;

	private:
		size_t longest_string__ = 0;
	};
}
