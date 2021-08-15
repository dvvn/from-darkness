#pragma once

// ReSharper disable CppInconsistentNaming

#define NOMINMAX
#define WIN32
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
//#define _CRT_SECURE_NO_WARNINGS
#define _HAS_DEPRECATED_ALLOCATOR_MEMBERS 0
#define _FSTREAM_SUPPORTS_EXPERIMENTAL_FILESYSTEM 0

//-----

namespace nstd::detail
{
	struct rt_assert_data
	{
		const char* file; //__FILE__
		const char* func; //__FUNCSIG__
		const char* line; //__LINE__
	};

	void rt_assert_helper(const char* expr, const char* msg, const char* file, const char* func, unsigned int line);

	constexpr const char* rt_assert_msg(const char* msg = nullptr)
	{
		return msg;
	}

	constexpr bool rt_assert_have_msg( )
	{
		return false;
	}

	template <size_t S>
	constexpr bool rt_assert_have_msg(const char (&msg)[S])
	{
		return msg[0] != '\0';
	}

	constexpr void rt_assert_caller(bool value, const char* expr, const char* msg, const char* file, const char* func, unsigned int line)
	{
		if (!value)
			return;
		// ReSharper disable once CppIfCanBeReplacedByConstexprIf
		if (__builtin_is_constant_evaluated( ))
			throw;

		// ReSharper disable once CppUnreachableCode
		rt_assert_helper(expr, msg, file, func, line);
	}

	// STRUCT TEMPLATE is_bounded_array
	template <class>
	inline constexpr bool expr_is_message = false;

	template <class _Ty, size_t _Nx>
	inline constexpr bool expr_is_message<_Ty[_Nx]> = true;
	template <class _Ty, size_t _Nx>
	inline constexpr bool expr_is_message<_Ty(&)[_Nx]> = true;
	template <class _Ty, size_t _Nx>
	inline constexpr bool expr_is_message<const _Ty(&)[_Nx]> = true;
}

#ifdef _DEBUG
#define runtime_assert(expr, ...)\
			nstd::detail::rt_assert_caller(\
				nstd::detail::expr_is_message<decltype(expr)> ?\
				!nstd::detail::rt_assert_have_msg(#__VA_ARGS__):\
				!!(expr) == false ,\
			#expr, nstd::detail::rt_assert_msg(__VA_ARGS__), __FILE__, __FUNCSIG__, __LINE__)
#else
#define runtime_assert(...) (void)0
#endif

#define _ASSERT runtime_assert
#define _ASSERTE runtime_assert
#define _ASSERT_EXPR runtime_assert
#include <crtdbg.h>

#include <concrt.h>
#undef _CONCRT_ASSERT
#define _CONCRT_ASSERT runtime_assert

#include <yvals.h>
#undef _STL_REPORT_ERROR
#define _STL_REPORT_ERROR runtime_assert
#undef _STL_VERIFY
#define _STL_VERIFY runtime_assert

//-----

#include <d3d9.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <winternl.h>
#include <Windows.h>

#include <any>
#include <filesystem>
#include <format>
#include <fstream>
#include <fstream>
#include <functional>
#include <iostream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>
#include <semaphore>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <future>
#include <variant>
#include <vector>
#include <queue>

#if defined(__cpp_lib_concepts)
#include <concepts>
#include <format>
#include <ranges>

namespace ranges
{
	namespace views = std::views;
	using namespace std::ranges;
}

#else

#define FMT_ASSERT runtime_assert
#include <fmt/format.h>
namespace std
{
	using fmt::format;
	using fmt::vformat;
}

#define RANGES_ASSERT runtime_assert
#define RANGES_ENSURE_MSG runtime_assert
#include <range/v3/all.hpp>

#endif

namespace std
{
	template <typename C>
	struct formatter<basic_ostringstream<C>>: formatter<typename basic_ostringstream<C>::_Mystr_view>
	{
		template <typename FormatContext>
		auto format(const basic_ostringstream<C>& v, FormatContext& ctx)
		{
			return formatter<typename basic_ostringstream<C>::_Mystr_view>::format(v.view( ), ctx);
		}
	};
}

#include <nlohmann/json.hpp>

#ifdef _DEBUG
#define TSL_DEBUG
#endif
#include <ordered map/include/tsl/ordered_map.h>
#include <ordered map/include/tsl/ordered_set.h>
#include <robin map/include/tsl/robin_map.h>
#include <robin map/include/tsl/robin_set.h>

#include <include/veque.hpp>

//#define FT2_BUILD_LIBRARY

#if !defined(_DEBUG) && !defined(CHEAT_GUI_TEST)
#define IMGUI_DISABLE_DEMO_WINDOWS
#endif
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS 1
#define IMGUI_USE_WCHAR32
#define IMGUI_DEBUG_PARANOID
#define IMGUI_DEFINE_MATH_OPERATORS
//#define IMGUI_ENABLE_FREETYPE
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#define IM_ASSERT runtime_assert
#define IM_ASSERT_USER_ERROR runtime_assert
#define ImDrawIdx size_t
// ReSharper disable CppWrongIncludesOrder
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include <misc/cpp/imgui_stdlib.h>
// ReSharper restore CppWrongIncludesOrder

//#include <WinReg/WinReg.hpp>

#include <thread_pool.hpp>

//not std
namespace nstd
{
	using ::thread_pool;

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

	template <typename T>
	struct equal_to: std::equal_to<T>
	{
	};

	template <class _Elem, class _Traits>
	struct equal_to<std::basic_string_view<_Elem, _Traits>>
	{
		_NODISCARD constexpr bool operator()(const std::basic_string_view<_Elem, _Traits>& _Left,
											 const std::basic_string_view<_Elem, _Traits>& _Right) const
		{
			return _Left == _Right;
		}

		using is_transparent = void;
	};

	template <class _Elem, class _Traits, class _Alloc>
	struct equal_to<std::basic_string<_Elem, _Traits, _Alloc>>: equal_to<std::basic_string_view<_Elem, _Traits>>
	{
	};

	template <typename T>
	struct hash: std::hash<T>
	{
	};

	template <class _Elem, class _Traits, class _Alloc>
	struct hash<std::basic_string<_Elem, _Traits, _Alloc>>: std::hash<std::basic_string_view<_Elem, _Traits>>
	{
	};

	template <typename T, typename A = std::allocator<T>>
	using deque = veque::veque<T, veque::fast_resize_traits, A>;

#ifdef TSL_ROBIN_MAP_H

	template <typename Key, typename Value,
			  typename Hasher = hash<Key>,
			  typename KeyEqual = equal_to<Key>,
			  typename Alloc = std::allocator<std::pair<Key, Value>>>
	using _Unordered_map = tsl::robin_map<Key, Value, Hasher, KeyEqual, Alloc>;

	template <typename Key, typename Value>
	using unordered_map = _Unordered_map<Key, Value>;
#endif

#ifdef TSL_ROBIN_SET_H

	template <typename Value,
			  typename Hasher = hash<Value>,
			  typename KeyEqual = equal_to<Value>,
			  typename Alloc = std::allocator<Value>>
	using _Unordered_set = tsl::robin_set<Value, Hasher, KeyEqual, Alloc>;

	template <typename Value>
	using unordered_set = _Unordered_set<Value>;
#endif

#ifdef TSL_ORDERED_MAP_H
	template <typename Key, typename Value,
			  template<typename ValueUsed, typename Alloc> typename Base,
			  typename ValueUsed = std::pair<Key, Value>,
			  typename Alloc = std::allocator<ValueUsed>>
	using _Ordered_map = tsl::ordered_map<Key, Value, hash<Key>, equal_to<Key>, Alloc, Base<ValueUsed, Alloc>>;

	template <typename K, typename T>
	using ordered_map = _Ordered_map<K, T, deque>;

#endif

#ifdef TSL_ORDERED_SET_H

	template <typename Key, typename Value,
			  template<typename ValueUsed, typename Alloc> typename Base,
			  typename ValueUsed = std::pair<Key, Value>,
			  typename Alloc = std::allocator<ValueUsed>>
	using _Ordered_set = tsl::ordered_set<Key, Value, hash<Key>, equal_to<Key>, Alloc, Base<ValueUsed, Alloc>>;

	template <typename K, typename T>
	using ordered_set = _Ordered_set<K, T, deque>;

#endif
}

namespace nstd
{
	namespace detail
	{
		template <typename T>
		struct as_string_or_view;

		template <class E, class Tr>
		struct as_string_or_view<std::basic_string_view<E, Tr>>
		{
			constexpr auto operator()(const std::basic_string_view<E, Tr>& val) const
			{
				using str = std::basic_string<E, Tr>;
				return str(val._Unchecked_begin( ), val._Unchecked_end( ));
			}
		};

		template <class E, class Tr, class A>
		struct as_string_or_view<std::basic_string<E, Tr, A>>
		{
			using string_type = std::basic_string<E, Tr, A>;

			constexpr const string_type& operator()(const string_type& val) const
			{
				return val;
			}

			constexpr string_type& operator()(string_type& val) const
			{
				return val;
			}

			constexpr string_type operator()(string_type&& val) const
			{
				return val;
			}
		};

		template <class C, size_t S>
		struct as_string_or_view<C[S]>
		{
			constexpr auto operator()(const C (&val)[S]) const
			{
				using str = std::basic_string_view<C>;
				return str(val, S - 1);
			}
		};

		template <class E, class Tr, class A>
		struct as_string_or_view<std::basic_ostringstream<E, Tr, A>>
		{
			using stream_type = std::basic_ostringstream<E, Tr, A>;
			using string_type = typename stream_type::_Mystr;
			using string_view_type = typename stream_type::_Mystr_view;

			constexpr string_view_type operator()(const stream_type& val) const
			{
				return val.view( );
			}

			constexpr string_view_type operator()(string_type& val) const
			{
				return val.str( );
			}

			constexpr string_type operator()(string_type&& val) const
			{
				return val.str( );
			}
		};

		struct as_string_impl
		{
			template <typename T, std::destructible Impl = as_string_or_view<std::remove_cvref_t<T>>>
			constexpr decltype(auto) operator()(T&& val) const
			{
				return std::invoke(Impl( ), std::forward<T>(val));
			}
		};
	}

	inline constexpr auto as_string = detail::as_string_impl( );

	namespace detail
	{
		struct checksum_impl
		{
			template <typename T>
				requires(std::invocable<as_string_impl, T>)
			size_t operator ()(T&& val) const
			{
				auto&& str  = as_string(std::forward<T>(val));
				auto&& hash = std::_Hash_array_representation(str._Unchecked_begin( ), str.size( ));
				return (hash);
			}

			template <typename T>
				requires(std::is_trivially_destructible_v<T>)
			size_t operator ()(const std::span<T>& vec) const
			{
				return (std::_Hash_array_representation(vec._Unchecked_begin( ), vec.size( )));
			}

			size_t operator()(const std::filesystem::path& p) const
			{
				if (exists(p))
				{
					auto ifs = std::ifstream(p);
					using itr_t = std::istreambuf_iterator<char>;
					if (!ifs.fail( ))
					{
						const auto tmp = std::vector(itr_t(ifs), itr_t( ));
						return std::invoke(*this, std::span(tmp));
					}
				}

				return 0;
			}
		};
	}

	inline constexpr auto checksum = detail::checksum_impl( );
}

using namespace std::literals;

#define _STRINGIZE_R(x) _CONCAT(R,_STRINGIZE(##(x)##))

#define CHEAT_UNUSED_ASSERT\
	runtime_assert("This function added only as example");\
	(void)this;

#define CHEAT_OUTPUT_DIR _STRINGIZE_R(VS_OutputDir)"\\"
#define CHEAT_SOLUTION_DIR _STRINGIZE_R(VS_SolutionDir)"\\"
#define CHEAT_NAME _STRINGIZE(VS_SolutionName)
#define CHEAT_DUMPS_DIR /*CHEAT_OUTPUT_DIR*/CHEAT_SOLUTION_DIR _STRINGIZE_R(.out\dumps\)
#define CHEAT_IMPL_DIR CHEAT_SOLUTION_DIR _STRINGIZE_R(impl\cheat\)

#define CHEAT_CURRENT_FILE_PATH\
	std::string_view(##__FILE__).substr(std::string_view(CHEAT_SOLUTION_DIR).size( ) + /*impl\\*/5)

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

#include "cheat/sdk/Color.hpp"
#include "cheat/sdk/QAngle.hpp"
#include "cheat/sdk/Vector.hpp"
#include "cheat/sdk/Vector2D.hpp"
#include "cheat/sdk/Vector4D.hpp"
#include "cheat/sdk/VMatrix.hpp"
