module;

#include <cheat/hooks/instance_fwd.h>

export module CHEAT_HOOK_MODULE(client, frame_stage_notify);

export CHEAT_HOOK_NAMESPACE(client, frame_stage_notify)
{
	CHEAT_HOOK_INSTANCE_FWD;
}