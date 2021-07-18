#pragma once

#include "cheat/gui/tools/animator.h"

namespace cheat::gui::widgets
{
	class group: public tools::widget_animator
	{
	public:

		group();
		
		void show( );

		void begin( );
		void end( );
	};
}
