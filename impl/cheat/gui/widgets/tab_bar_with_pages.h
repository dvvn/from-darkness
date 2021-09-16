#pragma once

#include "tab_bar.h"

#include "cheat/gui/objects/shared_label.h"

#include <memory>

namespace cheat::gui::tools
{
	class perfect_string;
}

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

		renderable* find_item(tools::perfect_string&& title);

	private:
		struct pages;
		std::unique_ptr<pages> pages_;
	};
}
