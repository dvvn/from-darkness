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
