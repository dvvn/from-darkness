#pragma once
#include "cheat/gui/objects/renderable object.h"

#include <memory>

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

		void   add_tab(const objects::shared_label& label);
		size_t selected( ) const;

		void make_size_static( );
		void make_size_auto( );

	private:
		void handle_size_change( );

	public:
		void make_horisontal( );
		void make_vertical( );

	private:
		void handle_direction_change( );

	public:
		bool is_horisontal( ) const;
		bool is_vertical( ) const;

		void render( ) override;

	private:
		class item;
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
