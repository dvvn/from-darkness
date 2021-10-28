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
	struct should_skip_animation_frame_impl final : service<should_skip_animation_frame_impl>
											 , dhooks::_Detect_hook_holder_t<__COUNTER__, bool(csgo::C_BaseAnimating::*)( )>
											 , gui::widgets::abstract_renderable
	{
		should_skip_animation_frame_impl( );

		void render( ) override;

	protected:
		nstd::address get_target_method_impl( ) const override;
		void callback(/*float current_time*/) override;
		load_result load_impl( ) noexcept override;

	private:
		bool override_return__    = false;
		bool override_return_to__ = false;
	};

	CHEAT_SERVICE_SHARE(should_skip_animation_frame);
}
