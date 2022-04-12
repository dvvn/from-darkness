module;

#include <cheat/hooks/instance_fwd.h>

export module CHEAT_HOOK_MODULE(client_mode, create_move);

export CHEAT_HOOK_NAMESPACE(client_mode, create_move)
{
	CHEAT_HOOK_INSTANCE_FWD;
}