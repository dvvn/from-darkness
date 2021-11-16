#pragma once
// ReSharper disable CppUnusedIncludeDirective
#include "impl.h"

#include <nstd/type name.h>
#include <nstd/one_instance.h>

#ifdef CHEAT_HAVE_CONSOLE
#include <format>
#endif

// ReSharper restore CppUnusedIncludeDirective

namespace cheat
{
	template <typename T, bool Root = false>
	struct service : basic_service
	{
#ifdef CEHAT_SERVICE_HAVE_NAME
		std::string_view name( ) const final
		{
			constexpr auto tmp   = nstd::type_name<T, "cheat">;
			constexpr auto dummy = std::string_view("_impl");
			if constexpr (tmp.ends_with(dummy))
				return tmp.substr(0, tmp.size( ) - dummy.size( ));
			else
				return tmp;
		}
#endif

		const std::type_info& type( ) const final
		{
			return typeid(T);
		}

		bool root_class( ) const final
		{
			return Root;
		}
	};
}

#define CHEAT_SERVICE_SHARE(_NAME_) \
	struct _NAME_ : cheat::service_shared<_NAME_##_impl>, nstd::one_instance<_NAME_> { }

#define CHEAT_SERVICE_RESULT(msg, ret)\
	CHEAT_CONSOLE_LOG(std::format("\"{}\" {}", ((basic_service*)this)->name(), msg));\
	co_return ret;

#define CHEAT_SERVICE_LOADED \
	{CHEAT_SERVICE_RESULT("loaded", true)}

#define CHEAT_SERVICE_NOT_LOADED(why) \
	{runtime_assert(why);\
	CHEAT_SERVICE_RESULT(std::format("not loaded. {}", _STRINGIZE(why)), false)}
