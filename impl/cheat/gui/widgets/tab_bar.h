#pragma once
#include "cheat/gui/objects/renderable object.h"

#include <memory>

namespace std
{
	template <class Fty>
	class function;
}

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
	class tab_bar final: public objects::renderable
	{
	public:
		tab_bar( );
		~tab_bar( ) override;

		size_t get_index(tools::perfect_string&& title) const;

		void   add_tab(const objects::shared_label& title);
		size_t get_selected_index( ) const;

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
		class item;
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
