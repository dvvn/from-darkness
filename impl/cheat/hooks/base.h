#pragma once
//#include "cheat/core/console.h"
#include "cheat/core/service.h"

#include <detour hook/hook_utils.h>

namespace cheat::hooks
{
	template <typename Proxy, typename Target>
	struct service_hook_proxy : service<Proxy>, dhooks::_Detect_hook_holder_t<Target>
	{
	protected:
		std::string_view name() const final { return nstd::type_name<Proxy, "cheat::hooks">; }
	};

#define CHEAT_SERVICE_HOOK_PROXY_RUN\
	if(!this->hook( ))\
		CHEAT_SERVICE_NOT_LOADED("Unable to setup hook",if constexpr(false))\
	if(!this->enable())\
		CHEAT_SERVICE_NOT_LOADED("Unable to enable hook",if constexpr(false))\
	CHEAT_SERVICE_LOADED

#if !CHEAT_SERVICE_INGAME
#define CHEAT_SERVICE_HOOK_PROXY_IMPL CHEAT_SERVICE_SKIPPED
#else
#define CHEAT_SERVICE_HOOK_PROXY_IMPL CHEAT_SERVICE_HOOK_PROXY_RUN
#endif

#define CHEAT_SERVICE_HOOK_PROXY_IMPL_SIMPLE(name)\
	service_base::load_result name::load_impl() noexcept\
	{\
		CHEAT_NETVARS_CONFIG_USED\
		CHEAT_SERVICE_HOOK_PROXY_IMPL;\
	}

#define CHEAT_SERVICE_HOOK_PROXY_IMPL_SIMPLE_ALWAYS_ON(name)\
	service_base::load_result name::load_impl() noexcept\
	{\
		CHEAT_SERVICE_HOOK_PROXY_RUN;\
	}

#define CHEAT_SERVICE_HOOK_PROXY_IMPL_SIMPLE_ALWAYS_OFF(name)\
	service_base::load_result name::load_impl() noexcept\
	{\
		CHEAT_SERVICE_SKIPPED;\
	}
}
