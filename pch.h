#pragma once

// ReSharper disable CppInconsistentNaming

#define NOMINMAX
#define WIN32
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
//#define _CRT_SECURE_NO_WARNINGS
#define _HAS_DEPRECATED_ALLOCATOR_MEMBERS 0
#define _FSTREAM_SUPPORTS_EXPERIMENTAL_FILESYSTEM 0

#define BOOST_USE_WINDOWS_H
#define BOOST_UNORDERED_USE_MOVE
#define BOOST_MOVE_USE_STANDARD_LIBRARY_MOVE
#define BOOST_ALL_NO_LIB
#define BOOST_THREAD_USES_BOOST_FUNCTIONAL
#define BOOST_THREAD_VERSION 5
#define BOOST_FILESYSTEM_NO_DEPRECATED

#if defined(_WINDLL) /*|| defined(_DLL)*/
#define BOOST_THREAD_BUILD_DLL
#else
#define BOOST_THREAD_BUILD_LIB
#endif

#ifdef _DEBUG
#define BOOST_ENABLE_ASSERT_HANDLER
#else
#define NDEBUG
#define BOOST_DISABLE_ASSERTS
#endif
//#define BOOST_ENABLE_ASSERT_HANDLER
//#ifndef _DEBUG
//#define NDEBUG
//#endif
#include <boost/assert.hpp>

#include <yvals.h>
#undef _STL_REPORT_ERROR
#define _STL_REPORT_ERROR BOOST_ASSERT
#undef _STL_VERIFY
#define _STL_VERIFY BOOST_ASSERT_MSG

#include <concrt.h>
#undef _CONCRT_ASSERT
#define _CONCRT_ASSERT BOOST_ASSERT

#define _STRINGIZE_R(x) _CONCAT(R,_STRINGIZE(##(x)##))

#ifdef _DEBUG
#undef _ASSERT_EXPR
#undef _ASSERT
#undef _ASSERTE
#define _ASSERT_EXPR BOOST_ASSERT_MSG
#define _ASSERT BOOST_ASSERT
#define _ASSERTE BOOST_ASSERT
#endif

#include <d3d9.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <winternl.h>

#define _THREAD_
#define _CHRONO_
#define _MUTEX_
#define _OPTIONAL_
#define _FILESYSTEM_
//#define _UNORDERED_MAP_

#ifdef _DEBUG
#define TSL_DEBUG
#endif
#include <ordered map/include/tsl/ordered_map.h>
#include <ordered map/include/tsl/ordered_set.h>
#include <robin map/include/tsl/robin_map.h>
#include <robin map/include/tsl/robin_set.h>

////#include <map>
////
//namespace std
//{
////#define _MAP_
//	//using map = tsl::ordered_map;
//
//	template <typename K, typename T,typename H, typename E>
//	using unordered_map = tsl::robin_map<K, T,H,E>;	
//}

#include <any>
#include <concepts>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <include/veque.hpp>

//#define FT2_BUILD_LIBRARY

//#define BOOST_CONTAINER_CONTAINER_VECTOR_HPP
#define BOOST_CONTAINER_STRING_HPP
#define BOOST_LEXICAL_CAST_INCLUDED
#define BOOST_MATH_TOOLS_BIG_CONSTANT_HPP
#define BOOST_FORMAT_FWD_HPP
#define BOOST_FORMAT_HPP
#define BOOST_SYSTEM_DETAIL_TO_STD_CATEGORY_HPP_INCLUDED
#define BOOST_RE_REGEX_HPP_INCLUDED
#define BOOST_RANGE_ADAPTOR_TOKENIZED_HPP
#define BOOST_STRING_VIEW_FWD_HPP
#define BOOST_STRING_VIEW_HPP

namespace boost
{
	namespace movelib
	{
#define BOOST_MOVE_UNIQUE_PTR_HPP_INCLUDED
		using std::unique_ptr;
#define BOOST_MOVE_MAKE_UNIQUE_HPP_INCLUDED
		using std::make_unique;
	}

#define BOOST_SMART_PTR_MAKE_UNIQUE_HPP
	using std::make_unique;

#define BOOST_SMART_PTR_SHARED_PTR_HPP_INCLUDED
	using std::shared_ptr;
#define BOOST_SMART_PTR_MAKE_SHARED_HPP_INCLUDED
	using std::make_shared;
#define BOOST_SMART_PTR_WEAK_PTR_HPP_INCLUDED
	using std::weak_ptr;
#define BOOST_SMART_PTR_ENABLE_SHARED_FROM_THIS_HPP_INCLUDED
	using std::enable_shared_from_this;

#define BOOST_CORE_ADDRESSOF_HPP
	using std::addressof;
#define BOOST_VARIANT_HPP
	using std::variant;
	using std::visit;

#define BOOST_ATOMIC_ATOMIC_HPP_INCLUDED_
	using std::atomic;
#define BOOST_ANY_INCLUDED
	using std::any;
	using std::any_cast;
#define BOOST_CORE_SWAP_HPP
	using std::swap;

	template <typename ...Args>
	_NODISCARD constexpr decltype(auto) bind(Args&&...args)
	{
#define BOOST_BIND_HPP_INCLUDED
		return std::bind_front(std::forward<Args>(args)...);
	}

	namespace csbl
	{
#define BOOST_CSBL_VECTOR_HPP
		using std::vector;
	}
#ifdef VEQUE_HEADER_GUARD
#define BOOST_CONTAINER_DEVECTOR_HPP
	template <typename T, typename Allocator = std::allocator<T>>
	using devector = veque::veque<T, veque::fast_resize_traits, Allocator>;
#endif
}

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

namespace std
{
#if 0
	template <typename T>
	boost::optional<T> move(boost::optional<T>& opt)
	{
		using rvalue = boost::optional<T>&&;
		boost::optional<T> dummy = static_cast<rvalue>(opt);
		opt.reset( );
		return static_cast<rvalue>(dummy);
	}

	template <typename T>
	boost::optional<T> move(boost::optional<T>&& opt)
	{
		return move(opt);
	}
#endif

	template <typename T>
	boost::optional<T> move(const boost::optional<T>& opt) = delete; //use swap

	using boost::optional;

	namespace filesystem
	{
		class path: public boost::filesystem::path
		{
		public:
			path( ) = default;

			path(boost::filesystem::path&& p): boost::filesystem::path(move(p))
			{
			}

			path(const boost::filesystem::path& p): boost::filesystem::path((p))
			{
			}
		};
	}
}

namespace boost::filesystem::path_traits
{
	template < >
	struct is_pathable<std::string_view>
	{
		static const bool value = true;
	};
	template < >
	struct is_pathable<std::wstring_view>
	{
		static const bool value = true;
	};
}

#include <boost/function.hpp>
#include <boost/make_default.hpp>
#include <boost/operators.hpp>
#include <boost/pfr.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>
#include <boost/thread/executors/scheduled_thread_pool.hpp>
#include <boost/winapi/basic_types.hpp>

namespace std::chrono
{
	using namespace boost::chrono;

	template <typename T>
	constexpr bool is_clock_v = true;
}

#include <semaphore>

//#include <boost/range.hpp>
//#include <boost/range/adaptors.hpp>
//#include <boost/range/algorithm.hpp>
//#include <boost/geometry.hpp>
//#include <boost/hana.hpp> outdated
//#include <boost/log/core.hpp>

#if 0
#define RANGES_ASSERT BOOST_ASSERT
#define RANGES_ENSURE_MSG BOOST_ASSERT_MSG
#include <range/v3/all.hpp>
#endif

#if !defined(_FORMAT_)
#define FMT_ASSERT BOOST_ASSERT_MSG
#include <fmt/format.h>
#endif

#if defined (NDEBUG) && !defined(CHEAT_GUI_TEST)
#define IMGUI_DISABLE_DEMO_WINDOWS
#endif
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS 1
#define IMGUI_USE_WCHAR32
#define IMGUI_DEBUG_PARANOID
#define IMGUI_DEFINE_MATH_OPERATORS
//#define IMGUI_ENABLE_FREETYPE
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#define IM_ASSERT BOOST_ASSERT
#define IM_ASSERT_USER_ERROR BOOST_ASSERT_MSG
#define ImDrawIdx size_t
// ReSharper disable CppWrongIncludesOrder
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>
// ReSharper restore CppWrongIncludesOrder

//#include <WinReg/WinReg.hpp>

namespace tsl::detail
{
	template <typename T>
	struct tsl_equal_to: std::equal_to<T>
	{
	};

	template <class _Elem, class _Traits>
	struct tsl_equal_to<std::basic_string_view<_Elem, _Traits>>
	{
		_NODISCARD constexpr bool operator()(const std::basic_string_view<_Elem, _Traits>& _Left,
											 const std::basic_string_view<_Elem, _Traits>& _Right) const
		{
			return _Left == _Right;
		}

		using is_transparent = void;
	};

	template <class _Elem, class _Traits, class _Alloc>
	struct tsl_equal_to<std::basic_string<_Elem, _Traits, _Alloc>>: tsl_equal_to<std::basic_string_view<_Elem, _Traits>>
	{
	};

	template <typename T>
	struct tsl_hash: std::hash<T>
	{
	};

	template <class _Elem, class _Traits, class _Alloc>
	struct tsl_hash<std::basic_string<_Elem, _Traits, _Alloc>>: std::hash<std::basic_string_view<_Elem, _Traits>>
	{
	};
}

namespace cheat::utl
{
#ifdef TSL_ROBIN_MAP_H
	template <typename K, typename T>
	using unordered_map = tsl::robin_map<K, T, tsl::detail::tsl_hash<K>, tsl::detail::tsl_equal_to<K>>;
#else
	using std::unordered_map;
#endif

#ifdef TSL_ROBIN_SET_H
	template <typename T>
	using unordered_set = tsl::robin_set<T, tsl::detail::tsl_hash<T>, tsl::detail::tsl_equal_to<T>>;
#else
	using std::unordered_set;
#endif

#ifdef TSL_ORDERED_MAP_H
	template <typename K, typename T>
	using ordered_map = tsl::ordered_map<K, T, tsl::detail::tsl_hash<K>, tsl::detail::tsl_equal_to<K>>;

	template <typename K, typename T, typename A = std::allocator<std::pair<K, T>>>
	using ordered_map_fast = tsl::ordered_map<K, T, tsl::detail::tsl_hash<K>, tsl::detail::tsl_equal_to<K>, A, std::vector<typename A::value_type, A>>;
#else
	//todo
#endif

#ifdef TSL_ORDERED_SET_H
	template <typename T>
	using ordered_set = tsl::ordered_set<T, tsl::detail::tsl_hash<T>, tsl::detail::tsl_equal_to<T>>;

	template <typename T, typename A = std::allocator<T>>
	using ordered_set_fast = tsl::ordered_set<T, tsl::detail::tsl_hash<T>, tsl::detail::tsl_equal_to<T>, A, std::vector<T, A>>;

#else
	//todo
#endif

	using boost::devector;

	using std::set;
	using std::map;
	using std::list;

	/*using Concurrency::concurrent_vector;
	using Concurrency::concurrent_unordered_map;
	using Concurrency::concurrent_unordered_set;*/

	using boost::concurrent::sync_queue;
	using boost::concurrent::queue_op_status;

	using boost::noncopyable;
	namespace property_tree = boost::property_tree;
	using std::unique_ptr;
	using std::make_unique;
	using std::weak_ptr;
	using std::shared_ptr;
	using std::make_shared;
	using std::enable_shared_from_this;
	using std::vector;
	using std::deque;

	using boost::optional;
	//constexpr auto& none = boost::none; //std::nullopt or boost::none
	//using none_t = boost::none_t;
	using boost::make_optional;

	using std::tuple; //boost tuple suck, no move support
	using std::make_tuple;
	using std::tuple_size_v;
	using std::tuple_element_t;
	using std::tie;
	using std::forward_as_tuple;
	using std::make_from_tuple;
	using std::tuple_cat;
	using std::apply;

	using std::variant;
	using std::visit;
	template <class ...Fs>
	struct overload: Fs...
	{
		template <class ...Ts>
		overload(Ts&& ...ts) : Fs{std::forward<Ts>(ts)}...
		{
		}

		using Fs::operator()...;
	};

	template <class ...Ts>
	// ReSharper disable once CppEnforceFunctionDeclarationStyle
	overload(Ts&&...) -> overload<std::remove_reference_t<Ts>...>;

	using std::span;
	using std::as_bytes;
	using std::string;
	using std::wstring;
	using std::basic_string;
	using std::string_view;
	using std::wstring_view;
	using std::basic_string_view;
	using std::pair;
	using std::make_pair;
	using std::atomic;
	using std::any;
	using std::array;
	using boost::function;
	using boost::function_traits;
	using std::invoke;
	using std::bind_front;
	using boost::thread;
	using boost::future;
	using boost::promise;
	using boost::async;
	using boost::launch;
	using boost::thread_interrupted;
	using boost::thread_exception;
	namespace future_state = boost::future_state;
	namespace this_thread = boost::this_thread;
	using boost::shared_future;
	using std::reference_wrapper;
	using std::ref;
	using std::cref;
	using thread_pool = boost::basic_thread_pool;
	using std::addressof;
	using std::forward;
	using std::move;
	using boost::make_default;
#ifdef FMT_VERSION
	using fmt::to_string;
	using fmt::to_wstring;
	using fmt::format;
	using fmt::vformat;
#else
	using std::to_string;
	using std::to_wstring;
	using std::format;
	using std::vformat;
#endif
	using std::swap;
	using std::get;
	using boost::mutex;
	using boost::shared_mutex;
	using boost::make_lock_guard;
	using std::binary_semaphore;
	using std::counting_semaphore;
	using namespace boost::core;

	namespace filesystem = boost::filesystem;
	namespace chrono = boost::chrono;
}

namespace ranges
{
#if 1
	//namespace actions= std::actions;
	namespace views = std::views;
	using namespace std::ranges;
#endif

#if 0
		namespace views = boost::adaptors;
	using namespace boost::range;
#endif
}

namespace std
{
#if 0
	template <typename C, typename Tr>
	struct hash<boost::basic_string_view<C, Tr>>
	{
		using value_type = boost::basic_string_view<C, Tr>;

		auto operator()(const value_type& str) const -> size_t
		{
			using std_sv = std::basic_string_view<C, Tr>;
			static_assert(sizeof(value_type) == sizeof(std_sv));
			return std::hash<std_sv>()(reinterpret_cast<const std_sv&>(str));
		}
	};
#endif

#if 0
	
	template <typename ...Ts>
	using _Multi_type = cheat::utl::multi_type<Ts...>;

	template <typename T, typename ...Ts>
	auto get(_Multi_type<Ts...>& page) -> T&
	{
		return page.template get<T>( );
	}

	template <size_t I, typename ...Ts>
	auto get(_Multi_type<Ts...>& page) -> decltype(auto)
	{
		return page.template get<I>( );
	}

	template <typename ...Ts>
	struct tuple_size<_Multi_type<Ts...>>: integral_constant<size_t, sizeof...(Ts)>
	{
	};

	template <size_t I, typename ...Ts>
	struct tuple_element<I, _Multi_type<Ts...>>: tuple_element<I, tuple<Ts...>>
	{
	};

#endif
}

//#define CHEAT_NETVARS_UPDATING

#define CHEAT_UNUSED_ASSERT\
	BOOST_ASSERT("This function added only as example");\
	(void)this;

#ifndef CHEAT_NETVARS_UPDATING
#if defined(NDEBUG) && !defined(CHEAT_HAVE_CONSOLE)//currently release mode used only for netvars/vtables dumping
#define CHEAT_NETVARS_UPDATING
#endif
#endif

#if 1//defined(_DEBUG) || defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_HAVE_CONSOLE
#endif

#define CHEAT_OUTPUT_DIR _STRINGIZE_R(VS_OutputDir)"\\"
#define CHEAT_SOLUTION_DIR _STRINGIZE_R(VS_SolutionDir)"\\"
#define CHEAT_NAME _STRINGIZE(VS_SolutionName)
#define CHEAT_DUMPS_DIR /*CHEAT_OUTPUT_DIR*/CHEAT_SOLUTION_DIR _STRINGIZE_R(.out\dumps\)
#define CHEAT_IMPL_DIR CHEAT_SOLUTION_DIR _STRINGIZE_R(impl\cheat\)

#define CHEAT_CURRENT_FILE_PATH\
	string_view(##__FILE__).substr(string_view(CHEAT_SOLUTION_DIR).size( ) + /*impl\\*/5)

namespace cheat::detail
{
	template <typename T, typename ...Ts>
	constexpr T _Bitflag_combine(Ts ...args)
	{
		if constexpr (sizeof...(Ts) == 0)
			return static_cast<T>(0);
		else
		{
			if constexpr (!std::is_enum_v<T>)
				return (static_cast<T>(args) | ...);
			else
			{
				auto tmp = _Bitflag_combine<std::underlying_type_t<T>>(args...);
				return static_cast<T>(tmp);
			}
		}
	}

	template <typename T, typename T1>
	constexpr bool _Bitflag_has(T enum_val, T1 val)
	{
		if constexpr (!std::is_enum_v<T>)
			return enum_val & static_cast<T>(val);
		else
			return _Bitflag_has(static_cast<std::underlying_type_t<T>>(enum_val), val);
	}

	template <typename T, typename T1>
	constexpr T _Bitflag_remove(T enum_val, T1 val)
	{
		if constexpr (!std::is_enum_v<T>)
			return enum_val & ~static_cast<T>(val);
		else
		{
			auto tmp = _Bitflag_remove(static_cast<std::underlying_type_t<T>>(enum_val), val);
			return static_cast<T>(tmp);
		}
	}

	template <typename T>
	constexpr auto _Bitflag_raw_type( )
	{
		if constexpr (std::is_enum_v<T>)
			return std::underlying_type_t<T>( );
		else
			return T( );
	}
}

#define CHEAT_ENUM_STRUCT_FILL(_NAME_, ...)\
	using value_type_raw = decltype(cheat::detail::_Bitflag_raw_type<value_type>());\
	\
	constexpr _NAME_( )=default;\
	constexpr _NAME_(value_type_raw val): value__(cheat::detail::_Bitflag_combine<value_type>(val)){ }\
	constexpr auto operator<=>(const _NAME_&) const = default;\
	\
	constexpr value_type value() const {return value__;}\
	constexpr value_type_raw value_raw() const {return static_cast<value_type_raw>(value__);}\
private:\
	value_type value__ = cheat::detail::_Bitflag_combine<value_type>(__VA_ARGS__);

#define CHEAT_ENUM_STRUCT_FILL_BITFLAG(_NAME_, ...)\
	using value_type_raw = decltype(cheat::detail::_Bitflag_raw_type<value_type>());\
	\
	constexpr auto operator<=>(const _NAME_&) const = default;\
	\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr _NAME_(T ... vals): value__(cheat::detail::_Bitflag_combine<value_type>(vals...)) { }\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr bool has(T ... vals) const { return (cheat::detail::_Bitflag_has(value__, vals) || ...); }\
	constexpr bool has(_NAME_ other) const{ return this->has(other.value__); }\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr bool has_all(T ... vals) const { return cheat::detail::_Bitflag_has(value__, cheat::detail::_Bitflag_combine<value_type>(vals...));}\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr _NAME_& add(T ... vals) { value__ = cheat::detail::_Bitflag_combine<value_type>(value__, vals...); return *this; }\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr _NAME_& remove(T ... vals) { value__ = cheat::detail::_Bitflag_remove<value_type>(value__, vals...); return *this; }\
	\
	constexpr value_type value() const {return value__;}\
	constexpr value_type_raw value_raw() const {return static_cast<value_type_raw>(value__);}\
private:\
	value_type value__ = cheat::detail::_Bitflag_combine<value_type>(__VA_ARGS__);

#include "cheat/hooks/_impl/hook_utils.h"
#include "cheat/utils/memory backup.h"
#include "cheat/utils/module info/module info.h"
#include "cheat/utils/winapi/threads.h"

#include "cheat/utils/Color.hpp"
#include "cheat/utils/QAngle.hpp"
#include "cheat/utils/Vector.hpp"
#include "cheat/utils/Vector2D.hpp"
#include "cheat/utils/Vector4D.hpp"
#include "cheat/utils/VMatrix.hpp"
