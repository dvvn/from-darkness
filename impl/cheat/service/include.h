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
	namespace detail
	{
#ifdef CEHAT_SERVICE_HAVE_NAME

		constexpr auto fix_service_name(const std::string_view& name)
		{
			auto str             = nstd::drop_namespace(name, "cheat");
			constexpr auto dummy = std::string_view("_impl");
			if (str.ends_with(dummy))
				str.erase(str.end( ) - dummy.size( ), str.end( ));
			return str;
		}

		template <typename T>
		_INLINE_VAR constexpr auto service_name = []
		{
			constexpr auto raw = nstd::type_name<T>;
			const auto raw_str = fix_service_name(raw);
#if 1
			constexpr auto buff_size = fix_service_name(raw).size( );
			auto buff                = nstd::string_to_buffer<buff_size>(raw_str);
#else
			auto buff = nstd::string_to_buffer<raw_str.size()>(raw_str);
#endif
			return buff;
		}( );

#endif
	}

	template <typename T, bool Root = false>
	struct service : basic_service
	{
#ifdef CEHAT_SERVICE_HAVE_NAME
		std::string_view name( ) const final
		{
			return detail::service_name<T>.view( );
			/*constexpr auto tmp   = nstd::type_name<T, "cheat">;
			constexpr auto dummy = std::string_view("_impl");
			if constexpr (tmp.ends_with(dummy))
				return tmp.substr(0, tmp.size( ) - dummy.size( ));
			else
				return tmp;*/
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
