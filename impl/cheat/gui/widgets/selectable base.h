#pragma once

#include "cheat/gui/tools/animator.h"

namespace cheat::gui::widgets
{
	class selectable_base
	{
	protected:
		selectable_base(bool selected = false);
	
	public:
		void select( );
		void deselect( );
		void toggle( );

		bool selected( ) const;
		bool animating( ) const;

	protected:
		bool Update( );
		float Anim_value()const;

	private:
		tools::animator anim__;
	};
}
