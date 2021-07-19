#pragma once
#include "widget animator.h"

namespace cheat::gui::widgets
{
	class group: public content_background_fader
	{
	public:
		group( );

		void show( );

		void begin( );
		void end( );
	};
}
