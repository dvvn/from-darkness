#pragma once

#include "cheat/gui/objects/renderable object.h"
#include "cheat/settings/shared_data.h"

#include <nstd/one_instance.h>

namespace cheat::features
{
	class aimbot final: public gui::objects::renderable
					  , public settings::shared_data
					  , public nstd::one_instance_shared<aimbot>
	{
	public:
		aimbot( );
		~aimbot( ) override;

		void render( ) override;
		void save(json& in) const override;
		void load(const json& out) override;
	};
}
