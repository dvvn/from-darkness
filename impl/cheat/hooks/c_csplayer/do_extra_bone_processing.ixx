module;

#include <cheat/hooks/instance_fwd.h>

export module CHEAT_HOOK_MODULE(c_csplayer, do_extra_bone_processing);

export CHEAT_HOOK_NAMESPACE(c_csplayer, do_extra_bone_processing)
{
	CHEAT_HOOK_INSTANCE_FWD;
}