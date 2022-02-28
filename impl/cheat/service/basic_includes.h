#pragma once

#include <nstd/runtime_assert.h>
#include <nstd/ranges.h>
#include <nstd/type_traits.h>

#include <cppcoro/static_thread_pool.hpp>
//#include <cppcoro/task.hpp>
#include <cppcoro/shared_task.hpp>
//#include <cppcoro/async_mutex.hpp>
#include <cppcoro/when_all.hpp>

#include <span>
#include <string>
#include <functional>
#include <vector>

//#define CHEAT_SERVICE_REGISTER(_NAME_) \
//	__pragma(message("Service \""#_NAME_"\" registered at " __TIME__))\
//	[[maybe_unused]]\
//	static const auto _CONCAT(_Reg_,_NAME_) = (::cheat::services_loader::get( ).deps().add(std::make_shared<_NAME_>( )), static_cast<std::byte>(0))

//#ifdef CHEAT_GUI_TEST
//#define CHEAT_SERVICE_REGISTER(_NAME_) __pragma(message("Service \""#_NAME_"\" is disabled"))
//#else
//#define CHEAT_SERVICE_REGISTER CHEAT_SERVICE_REGISTER
//#endif

#if 0

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
	struct _NAME_ : cheat::shared_service<_NAME_##_impl>, nstd::one_instance<_NAME_> { }

#define CHEAT_SERVICE_RESULT(msg, ret)\
	CHEAT_CONSOLE_LOG(std::format("\"{}\" {}", ((basic_service*)this)->name(), msg));\
	co_return ret;

#define CHEAT_SERVICE_LOADED \
	{CHEAT_SERVICE_RESULT("loaded", true)}

#define CHEAT_SERVICE_NOT_LOADED(why) \
	{runtime_assert(why);\
	CHEAT_SERVICE_RESULT(std::format("not loaded. {}", _STRINGIZE(why)), false)}

#endif