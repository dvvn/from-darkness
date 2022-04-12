module;

#include <cheat/hooks/instance_fwd.h>

export module CHEAT_HOOK_MODULE(winapi, wndproc);

export CHEAT_HOOK_NAMESPACE(winapi, wndproc)
{
	CHEAT_HOOK_INSTANCE_FWD;
}