module;

#include <cheat/hooks/instance_fwd.h>

export module CHEAT_HOOK_MODULE(studio_render, draw_model);

export CHEAT_HOOK_NAMESPACE(studio_render, draw_model)
{
	CHEAT_HOOK_INSTANCE_FWD;
}