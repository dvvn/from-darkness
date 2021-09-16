#pragma once

#include "cheat/core/service.h"
#include "cheat/gui/objects/renderable object.h"
#include "cheat/settings/shared_data.h"

#include <nstd/one_instance.h>

namespace cheat::features
{
	class aimbot final: public service<aimbot>
					  , public gui::objects::renderable
					  , public settings::shared_data

	{
	public:
		aimbot( );
		~aimbot( ) override;

		aimbot(aimbot&&) noexcept;
		aimbot& operator=(aimbot&&) noexcept;

		void render( ) override;
		void save(json& in) const override;
		void load(const json& out) override;

	protected:
		load_result load_impl( ) override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
