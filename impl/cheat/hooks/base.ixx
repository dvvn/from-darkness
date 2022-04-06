module;

#include <cstdint>

export module cheat.hooks.base;
import dhooks.wrapper;
export import cheat.console.object_message;

#define CHEAT_HOOKS_CONSOLE_LOG_FN(_FN_NAME_)\
bool _FN_NAME_( ) final\
{\
	const auto ret = dhooks::hook_holder_data::_FN_NAME_( );\
	console::object_message<base>(ret ? #_FN_NAME_##"ed" : "not "#_FN_NAME_##"ed");\
	return ret;\
}

export namespace cheat::hooks
{
	template<typename Fn, size_t Idx = 0>
	struct base :dhooks::select_hook_holder<Fn, Idx>, private console::object_message_auto<base<Fn, Idx>>
	{
		CHEAT_HOOKS_CONSOLE_LOG_FN(hook);
		CHEAT_HOOKS_CONSOLE_LOG_FN(enable);
		CHEAT_HOOKS_CONSOLE_LOG_FN(disable);
	};
}