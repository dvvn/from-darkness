#pragma once

#include "cheat/core/service.h"
#include "cheat/gui/widgets/absrtact_renderable.h"
#include "cheat/settings/shared_data.h"

namespace cheat::features
{
	class aimbot_impl final : public service<aimbot_impl>
							, public gui::widgets::abstract_renderable
							, public settings::shared_data

	{
	public:
		aimbot_impl( );
		~aimbot_impl( ) override;

		aimbot_impl(aimbot_impl&&) noexcept;
		aimbot_impl& operator=(aimbot_impl&&) noexcept;

		void render( ) override;
		void save(json& in) const override;
		void load(const json& out) override;

	protected:
		load_result load_impl( ) noexcept override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	CHEAT_SERVICE_SHARE(aimbot);
}
