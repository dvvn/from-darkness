#pragma once
#include "cheat/core/settings.h"

namespace cheat::features
{
	class anti_aim final: public gui::menu::empty_page, public settings_data, public utl::one_instance<anti_aim>
	{
	public:
		anti_aim( );

		auto render( ) -> void override;
		auto update( ) -> void override;

	private:
		bool test__ = true;
	};
}
