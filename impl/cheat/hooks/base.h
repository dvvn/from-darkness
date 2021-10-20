#pragma once
//#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"

#include <dhooks/hook_utils.h>

namespace cheat::hooks
{
	template <typename Proxy/*, typename ...TargetTypes*/>
	struct service_hook_proxy : service<Proxy> //, dhooks::hook_holder<TargetTypes...>
	{
		std::string_view name( ) const final { return nstd::type_name<Proxy, "cheat", "hooks">; }
		std::string_view debug_type( ) const override { return "Hook"; }
	};
}

#define CHEAT_SETUP_HOOK_PROXY(_PROXY_,_TARGET_,...) \
	struct _PROXY_ final: service_hook_proxy<_PROXY_>, DHOOKS_DETECT_HOOK_HOLDER(std::declval<_TARGET_>( )),##__VA_ARGS__

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

namespace cheat::hooks
{
	namespace detail
	{
		template <typename T>
		constexpr decltype(auto) unwrap_invocable(T&& obj)
		{
			if constexpr (!std::invocable<decltype(obj)>)
				return std::forward<T>(obj);
			else
				return std::invoke((obj));
		};
	}

	template <typename Source, typename Index>
	nstd::address get_target_method_helper(Source&& src, Index&& idx)
	{
		decltype(auto) src_unw = detail::unwrap_invocable(std::forward<Source>(src));
		decltype(auto) idx_unw = detail::unwrap_invocable(std::forward<Index>(idx));

		const auto index_to_number = [&]
		{
			using idx_t = std::remove_cvref_t<decltype(idx_unw)>;
			if constexpr (std::convertible_to<idx_t, size_t>)
				return static_cast<size_t>(idx_unw);
			else if constexpr (std::derived_from<idx_t, nstd::address>)
				return idx_unw.value( );
		};

		const auto get_vfunc = [&]<typename C>(C* instance)
		{
			auto vtable = dhooks::_Pointer_to_virtual_class_table(instance);
			auto index  = index_to_number( );
			return vtable[index];
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
