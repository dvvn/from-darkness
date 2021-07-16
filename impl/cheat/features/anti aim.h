#pragma once
#include "cheat/settings/settings.h"

namespace cheat::features
{
	class anti_aim final: public gui::objects::empty_page, public settings_data, public utl::one_instance<anti_aim>
	{
	public:
		anti_aim( );

		void render( ) override;
		void update( ) override;

	private:
		bool test__ = true;
	};
}
