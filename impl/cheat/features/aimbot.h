#pragma once

#if 0


#include "cheat/gui/widgets/absrtact_renderable.h"
#include "cheat/service/include.h"
#include "cheat/settings/shared_data.h"

namespace cheat::features
{
	class aimbot_impl final : public dynamic_service<aimbot_impl>
							, public gui::widgets::abstract_renderable
							, public settings::shared_data

	{
	public:
		aimbot_impl( );
		~aimbot_impl( ) override;

		aimbot_impl(aimbot_impl&&) noexcept;
		aimbot_impl& operator=(aimbot_impl&&) noexcept;

		bool render( ) override;
		void save(json& in) const override;
		void load(const json& out) override;

	protected:
		bool load_impl( ) noexcept override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

}
#endif