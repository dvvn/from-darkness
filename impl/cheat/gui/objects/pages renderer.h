#pragma once
#include "abstract page.h"

namespace cheat::gui::objects
{
	class vertical_pages_renderer final: public abstract_pages_renderer
	{
	public:
		void render( ) override;
		void init( ) override;

	private:
		size_t longest_string__ = 0;
	};

	class horizontal_pages_renderer final: public abstract_pages_renderer
	{
	public:
		void render( ) override;
		void init( ) override;

	private:
		size_t chars_count__ = 0;
	};
}
