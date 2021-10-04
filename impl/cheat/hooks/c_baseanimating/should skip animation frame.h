#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

#include "cheat/gui/widgets/absrtact_renderable.h"

namespace cheat::csgo
{
	class C_BaseAnimating;
}

namespace cheat::hooks::c_base_animating
{
	class should_skip_animation_frame final: public service_hook_proxy<should_skip_animation_frame, bool(csgo::C_BaseAnimating::*)( )>
										   , public gui::widgets::abstract_renderable
	{
	public :
		should_skip_animation_frame( );

		void render( ) override;

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(/*float current_time*/) override;
		load_result load_impl() noexcept override;

	private:
		bool override_return__    = false;
		bool override_return_to__ = false;
	};
}
