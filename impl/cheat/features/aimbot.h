#pragma once

#include "cheat/gui/objects/renderable object.h"
#include "cheat/gui/objects/shared_label.h"
#include "cheat/settings/settings_shared.h"

#include <nstd/one_instance.h>

namespace cheat::features
{
	class aimbot final: public gui::objects::renderable
					  , public gui::objects::non_abstract_label
					  , public settings_shared
					  , public nstd::one_instance_shared<aimbot>
	{
	public:
		aimbot( );
		~aimbot( ) override;

		void              render( ) override;
		void              save(json& in) const override;
		void              load(const json& out) override;
		std::wstring_view title( ) const override;
	};
}
