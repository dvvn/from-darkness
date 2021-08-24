#pragma once
#include "cheat/settings/settings.h"

namespace cheat::features
{
	class aimbot final: public gui::objects::empty_page, public settings_data, public nstd::one_instance<aimbot>
	{
	public:
		aimbot( );

		void render( ) override;
		void update( ) override;

	private:
		bool test__ = true;
	};
}
