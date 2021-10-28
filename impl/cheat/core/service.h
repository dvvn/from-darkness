#pragma once
#include "service_impl.h"

#include <nstd/type name.h>

namespace cheat
{
	template <typename T>
	struct service : basic_service
	{
		std::string_view name( ) const final { return nstd::type_name<T, "cheat">; }
		const std::type_info& type( ) const final { return typeid(T); }
	};
}

#define CHEAT_SERVICE_SHARE(_NAME_) \
	struct _NAME_ : service_shared<_NAME_##_impl>, nstd::one_instance<_NAME_> { }

#define CHEAT_SERVICE_ADD_SHARED_DEPENDENCY(_NAME_) \
	this->add_dependency(_NAME_::get())

#define CHEAT_SERVICE_RESULT(msg, ret)\
	CHEAT_CONSOLE_LOG(std::format("\"{}\" {}", ((basic_service*)this)->name(), msg));\
	co_return ret;

#define CHEAT_SERVICE_LOADED \
	{CHEAT_SERVICE_RESULT("loaded", true)}

#define CHEAT_SERVICE_NOT_LOADED(why) \
	{runtime_assert(why);\
	CHEAT_SERVICE_RESULT(std::format("not loaded. {}", _STRINGIZE(why)), false)}
