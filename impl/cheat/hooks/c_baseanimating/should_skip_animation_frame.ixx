module;

#include <cheat/hooks/instance_fwd.h>

export module CHEAT_HOOK_MODULE(c_base_animating, should_skip_animation_frame);

export CHEAT_HOOK_NAMESPACE(c_base_animating, should_skip_animation_frame)
{
	CHEAT_HOOK_INSTANCE_FWD;
}