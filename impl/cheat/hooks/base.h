#pragma once
//#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"

#include <detour hook/hook_utils.h>

namespace cheat::hooks
{
	template <typename Proxy, typename Target>
	struct service_hook_proxy : service<Proxy>, dhooks::_Detect_hook_holder_t<Target>
	{
		std::string_view name() const final { return nstd::type_name<Proxy, "cheat::hooks">; }
		service_base::debug_info_t debug_info() const final { return {"Hook", "set", "unused", "NOT set"}; }
	};
}

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CHEAT_HOOK_PROXY_INIT_IMPL \
if(!this->hook( )) \
	CHEAT_SERVICE_NOT_LOADED("Unable to setup hook") \
if(!this->enable()) \
	CHEAT_SERVICE_NOT_LOADED("Unable to enable hook") \
CHEAT_SERVICE_LOADED

#define CHEAT_HOOK_PROXY_INIT_0 CHEAT_SERVICE_SKIPPED
#define CHEAT_HOOK_PROXY_INIT_1 CHEAT_HOOK_PROXY_INIT_IMPL
#define CHEAT_HOOK_PROXY_INIT_X(_NUM_) CHEAT_HOOK_PROXY_INIT_##_NUM_//_CONCAT does not work, idk why

#define CHEAT_HOOK_PROXY_INIT(PX) CHEAT_HOOK_PROXY_INIT_X(PX)

#define CHEAT_HOOK_PROXY_INIT_FN(_NAME_, _ACTIVE_) \
service_base::load_result _NAME_::load_impl() noexcept\
{\
	CHEAT_NETVARS_CONFIG_USED\
	CHEAT_HOOK_PROXY_INIT(_ACTIVE_)\
}

#define CHEAT_HOOK_PROXY_CALLBACK_BLOCKER\
	runtime_assert("Skipped but called");\
	__pragma(message(__FUNCTION__": skipped"))

namespace cheat::hooks
{
	namespace detail
	{
		template <typename T>
		constexpr auto unwrap_invocable(const T& obj)
		{
			if constexpr (!std::invocable<T>)
				return obj;
			else
				return std::invoke(obj);
		};
	}

	template <typename Source, typename Index>
	nstd::address get_target_method_helper(const Source& src, const Index& idx)
	{
		auto src_unw = detail::unwrap_invocable(src);
		auto idx_unw = detail::unwrap_invocable(idx);

		constexpr auto get_vfunc = [&]<typename C>(C* instance)
		{
			return dhooks::_Pointer_to_virtual_class_table(instance)[idx_unw];
		};

		if constexpr (std::invocable<decltype(src_unw), csgo_interfaces*>)
			return get_vfunc(std::invoke(src_unw, csgo_interfaces::get_ptr( )).get( ));
		else
			return get_vfunc(src_unw);
	}

	template <typename Source>
	nstd::address get_target_method_helper(const Source& src)
	{
		return detail::unwrap_invocable(src);
	}
}

#define CHEAT_HOOK_PROXY_TARGET_FN(_NAME_,...) \
nstd::address _NAME_::get_target_method_impl() const\
{\
	return get_target_method_helper(__VA_ARGS__);\
}

//using namespace nstd::address_pipe; manualy
#define CHEAT_HOOK_PROXY_TARGET_PTR_FN(_NAME_, _HOLDER_, _SIG_, ...) \
nstd::address _NAME_::get_target_method_impl() const\
{\
	return CHEAT_FIND_SIG(_HOLDER_,_SIG_,__VA_ARGS__);\
}
