module;

//#include <cstdint>
#if 0
export module cheat.hooks.base;
export import cheat.console.object_message;
export import dhooks.entry;
export import nstd.one_instance;

import dhooks.wrapper;

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
#endif

#include <string_view>

export module cheat.hooks.base;
export import nstd.one_instance;

export namespace cheat::hooks
{
	class  base
	{
	public:
		virtual ~base( ) = default;

		virtual bool enable( ) = 0;
		virtual bool disable( ) = 0;

		virtual bool is_static( ) const noexcept = 0;
	};

	class class_base : public virtual base
	{
	public:
		bool is_static( ) const noexcept final
		{
			return false;
		}

		virtual std::string_view class_name( ) const noexcept = 0;
		virtual std::string_view function_name( ) const noexcept = 0;
	};

	class static_base : public virtual base
	{
	public:
		bool is_static( ) const noexcept final
		{
			return true;
		}

		virtual std::string_view function_name( ) const noexcept = 0;
	};
}