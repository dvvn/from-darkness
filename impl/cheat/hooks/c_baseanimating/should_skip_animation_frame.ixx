module;

export module cheat.hooks.c_base_animating:should_skip_animation_frame;
import cheat.csgo.interfaces.C_BaseAnimating;
import dhooks;

//namespace cheat::csgo
//{
//	class C_BaseAnimating;
//}

namespace cheat::hooks::c_base_animating
{
	export class should_skip_animation_frame final :public dhooks::select_hook_holder<bool(csgo::C_BaseAnimating::*)()>
	{
	public:
		should_skip_animation_frame( );

	protected:
		void callback(/*float current_time*/) override;

	private:
		//bool override_return__ = false;
		//bool override_return_to__ = false;
	};
}
