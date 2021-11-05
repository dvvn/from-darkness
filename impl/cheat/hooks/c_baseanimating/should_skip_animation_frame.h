#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class C_BaseAnimating;
}

namespace cheat::hooks::c_base_animating
{
	struct should_skip_animation_frame_impl final : service<should_skip_animation_frame_impl>
												  , dhooks::_Detect_hook_holder_t<__COUNTER__, bool(csgo::C_BaseAnimating::*)( )>
	{
		should_skip_animation_frame_impl( );

	protected:
		void* get_target_method( ) const override;
		void callback(/*float current_time*/) override;
		load_result load_impl( ) noexcept override;

	private:
		bool override_return__    = false;
		bool override_return_to__ = false;
	};

	CHEAT_SERVICE_SHARE(should_skip_animation_frame);
}
