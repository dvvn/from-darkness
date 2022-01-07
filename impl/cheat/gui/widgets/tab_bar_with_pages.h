#pragma once

#if 0

#include "tab_bar.h"

#include <memory>



namespace cheat::gui::widgets
{
	class tab_bar_with_pages final: public tab_bar
	{
		using tab_bar::add_tab;

	public:
		tab_bar_with_pages( );
		~tab_bar_with_pages( ) override;

		void render( ) override;

		void add_item(std::unique_ptr<tab_bar_item>&& bar_item, const objects::renderable_shared& data);

		renderable* find_item(const tools::cached_text::label_type& title);

	private:
		struct pages;
		std::unique_ptr<pages> pages_;
	};
}
#endif