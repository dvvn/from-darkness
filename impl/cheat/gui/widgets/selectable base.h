#pragma once

#include "widget animator.h"

namespace cheat::gui::widgets
{
	//not a selectable. base to selectable, checkbox, ant all same widgets
	class selectable_base: public widget_animator
	{
	protected:
		selectable_base(bool selected = false);

	public:
		void select( );
		void deselect( );
		void toggle( );

		bool selected( ) const;
	};
}
