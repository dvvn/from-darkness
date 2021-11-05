#pragma once
#include "impl.h"

#ifdef _DEBUG
#include <nstd/type name.h>
#endif

namespace cheat
{
	template <typename T>
	struct service : basic_service
	{
		std::string_view name( ) const final
		{
#ifdef _DEBUG
			constexpr auto tmp   = nstd::type_name<T, "cheat">;
			constexpr auto dummy = std::string_view("_impl");
			if constexpr (tmp.ends_with(dummy))
				return tmp.substr(0, tmp.size( ) - dummy.size( ));
			else
				return tmp;

#else
			std::terminate();
#endif
		}

		const std::type_info& type( ) const final
		{
			return typeid(T);
		}
	};
}

#define CHEAT_SERVICE_SHARE(_NAME_) \
	struct _NAME_ : service_shared<_NAME_##_impl>, nstd::one_instance<_NAME_> { }

#define CHEAT_SERVICE_RESULT(msg, ret)\
	CHEAT_CONSOLE_LOG(std::format("\"{}\" {}", ((basic_service*)this)->name(), msg));\
	co_return ret;

#define CHEAT_SERVICE_LOADED \
	{CHEAT_SERVICE_RESULT("loaded", true)}

#define CHEAT_SERVICE_NOT_LOADED(why) \
	{runtime_assert(why);\
	CHEAT_SERVICE_RESULT(std::format("not loaded. {}", _STRINGIZE(why)), false)}
