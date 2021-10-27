#pragma once
//#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"

#include <dhooks/hook_utils.h>

namespace cheat::hooks
{
	template <typename Proxy, size_t Idx, typename Fn>
	struct hook_instance_shared : service_instance_shared<Proxy>, dhooks::_Detect_hook_holder_t<Idx, Fn>
	{
	};
}

#define CHEAT_LOAD_HOOK_PROXY \
if(!this->hook( )) \
	CHEAT_SERVICE_NOT_LOADED("Unable to setup hook"); \
if(!this->enable()) \
	CHEAT_SERVICE_NOT_LOADED("Unable to enable hook"); \
CHEAT_SERVICE_LOADED

#define CHEAT_HOOK_PROXY_INIT_0 CHEAT_SERVICE_SKIPPED
#define CHEAT_HOOK_PROXY_INIT_1 CHEAT_HOOK_PROXY_INIT_IMPL
#define CHEAT_HOOK_PROXY_INIT_X(_NUM_) CHEAT_HOOK_PROXY_INIT_##_NUM_//_CONCAT does not work, idk why

#define CHEAT_HOOK_PROXY_INIT(...) CHEAT_LOAD_HOOK_PROXY

#define CHEAT_HOOK_PROXY_INIT_FN(_NAME_, _ACTIVE_) \
service_impl::load_result _NAME_::load_impl() noexcept\
{\
	CHEAT_NETVARS_CONFIG_USED\
	CHEAT_HOOK_PROXY_INIT(_ACTIVE_)\
}

#define CHEAT_HOOK_PROXY_TARGET_FN_0(...) \
	CHEAT_CALL_BLOCKER\
	return nullptr;
#define CHEAT_HOOK_PROXY_TARGET_FN_1(...) \
	return get_target_method_helper(__VA_ARGS__);
#define CHEAT_HOOK_PROXY_TARGET_FN_X(_HOLDER_, ...) \
	_CONCAT(CHEAT_HOOK_PROXY_TARGET_FN_,_HOLDER_)(__VA_ARGS__)

#define CHEAT_HOOK_PROXY_TARGET_FN(_NAME_, _HOLDER_, ...) \
nstd::address _NAME_::get_target_method_impl() const\
{\
	CHEAT_HOOK_PROXY_TARGET_FN_X(_HOLDER_, __VA_ARGS__)\
}
