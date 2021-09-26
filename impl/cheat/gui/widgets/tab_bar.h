#pragma once
#include "selectable.h"

namespace cheat::gui::tools
{
	class imgui_string;
	class imgui_string_transparent;
}


namespace cheat::gui::widgets
{
	struct tab_bar_item final: selectable
	{
	};

	class tab_bar: public objects::renderable
	{
	public:
		tab_bar( );
		~tab_bar( ) override;

		tab_bar_item* find_tab(const tools::cached_text::label_type& title);
		size_t find_tab_index(const tab_bar_item* item)const;

		void          add_tab(std::unique_ptr<tab_bar_item>&& item);
		tab_bar_item* get_selected( );

		//using sort_pred = std::function<bool (const tools::string_wrapper&, const tools::string_wrapper&)>;
		//void sort(const sort_pred& pred);

		size_t size( ) const;
		bool   empty( ) const;

		//----

		void make_size_static( );
		void make_size_auto( );

		void make_horisontal( );
		void make_vertical( );

		bool is_horisontal( ) const;
		bool is_vertical( ) const;

		void render( ) override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
