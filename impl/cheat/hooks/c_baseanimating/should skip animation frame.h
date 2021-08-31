#pragma once

#include "cheat/core/service.h"
#include "cheat/gui/objects/abstract page.h"
#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class C_BaseAnimating;
}

namespace cheat::hooks::c_base_animating
{
	class should_skip_animation_frame final: public base<should_skip_animation_frame,bool(csgo::C_BaseAnimating::*)()>
										   , public gui::objects::empty_page
										   , service_sometimes_skipped
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
