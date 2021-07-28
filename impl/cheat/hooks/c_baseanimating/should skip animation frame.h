#pragma once

#include "cheat/core/service.h"
#include "cheat/gui/objects/abstract page.h"
#include "cheat/sdk/entity/C_BaseAnimating.h"

namespace cheat::hooks::c_base_animating
{
	class should_skip_animation_frame final: public service<should_skip_animation_frame>,
											 public gui::objects::empty_page,
											 public decltype(_Detect_hook_holder(&csgo::C_BaseAnimating::ShouldSkipAnimationFrame))
	{
	public :
		should_skip_animation_frame( );

		void render( ) override;
	protected:
		bool Do_load( ) override;
		void Callback(/*float current_time*/) override;

	private:
		bool override_return__ = false;
		bool override_return_to__ = false;
	};
}
