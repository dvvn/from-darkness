module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.c_base_animating:should_skip_animation_frame;
import cheat.hooks.base;
import cheat.csgo.interfaces.C_BaseAnimating;

//namespace cheat::csgo
//{
//	class C_BaseAnimating;
//}

namespace cheat::hooks::c_base_animating
{
	export class should_skip_animation_frame final :public hook_base<should_skip_animation_frame, bool(csgo::C_BaseAnimating::*)()>
	{
	public:
		should_skip_animation_frame( );

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
		void callback(/*float current_time*/) override;

	private:
		bool override_return__ = false;
		bool override_return_to__ = false;
	};
}
