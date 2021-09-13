#pragma once
#include "selectable.h"

namespace cheat::gui::tools
{
	class string_wrapper;
	class perfect_string;
}

namespace cheat::gui::objects
{
	class abstract_label;
	using shared_label = std::shared_ptr<abstract_label>;
}

namespace cheat::gui::widgets
{
	struct tab_bar_item final: selectable
	{
	};

	class tab_bar : public objects::renderable
	{
	public:
		tab_bar( );
		~tab_bar( ) override;

		tab_bar_item* find_tab(tools::perfect_string&& title);

		tab_bar_item& add_tab(tools::string_wrapper&& title);
		tab_bar_item* get_selected( );

		tab_bar_item* begin( );
		tab_bar_item* end( );

		using sort_pred = std::function<bool (const tools::string_wrapper&, const tools::string_wrapper&)>;

		void sort(const sort_pred& pred);

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
