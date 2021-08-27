#pragma once

#include "cheat/core/service.h"
#include "cheat/gui/objects/abstract page.h"
#include "cheat/sdk/entity/C_BaseAnimating.h"

namespace cheat::hooks::c_base_animating
{
	class should_skip_animation_frame final: public service<should_skip_animation_frame>
										   , public dhooks::_Detect_hook_holder_t<decltype(&csgo::C_BaseAnimating::ShouldSkipAnimationFrame)>
										   , public gui::objects::empty_page
										   , service_hook_helper
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
										   , service_always_skipped
#endif
	{
	public :
		should_skip_animation_frame( );

		void render( ) override;

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(/*float current_time*/) override;

	private:
		bool override_return__    = false;
		bool override_return_to__ = false;
	};
}
