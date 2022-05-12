module;

#include <nstd/format.h>

#include <string_view>
#include <functional>
//#include <sstream>

export module cheat.console;
import nstd.text.convert;

struct string_accepter
{
	template<typename ...Ts>
	string_accepter(const std::basic_string_view<Ts...>)
	{
	}

	template<typename ...Ts>
	string_accepter(const std::basic_string<Ts...>)
	{
	}

	template<typename T>
	string_accepter(const T*)
	{
	}

	template<typename T, size_t S>
	string_accepter(const T(&)[S])
	{
	}
};

template<typename T>
constexpr bool is_trivial_object_v = std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(uintptr_t) * 4;//* 4 -> 16 or 32 bits limit

template<typename To, typename From>
decltype(auto) _Convert_to(From&& obj) noexcept
{
	if constexpr(!std::constructible_from<string_accepter, From>)
	{
		return std::forward<From>(obj);
	}
	else
	{
		using char_t = std::remove_cvref_t<decltype(obj[0])>;
		if constexpr(!std::same_as<To, char_t>)
			return nstd::text::convert_to<To>(std::basic_string_view<char_t>(obj));
		else if constexpr(std::is_rvalue_reference_v<decltype(obj)>)
			return From(std::forward<From>(obj));
		else
			return obj;
	}
}

template<typename T, typename Base, typename Adaptor>
class formatter_froxy : public Base
{
	[[no_unique_address]]
	Adaptor adp_;

public:
	template<typename T1, class FormatContext>
		requires(!is_trivial_object_v<T>)
	auto format(T1&& obj, FormatContext& fc) const
	{
		return Base::format(adp_(std::forward<T1>(obj)), fc);
	}

	template<class FormatContext>
		requires(is_trivial_object_v<T>)
	auto format(T obj, FormatContext& fc) const
	{
		return Base::format(adp_(std::move(obj)), fc);
	}
};

//template<typename CharT>
//struct convert_adaptor
//{
//	template<typename Q>
//	auto operator()(Q&& str) const
//	{
//		return _Convert_to<CharT>(std::forward<Q>(str));
//	}
//};

template<typename CharT>
struct invoke_adaptor
{
	template<typename ...Ts>
	decltype(auto) operator()(Ts&&...args) const
	{
		return _Convert_to<CharT>(std::invoke(std::forward<Ts>(args)...));
	}
};

template<typename T>
using invoke_result_fmt = std::remove_cvref_t<decltype(std::invoke(std::declval<T>( )))>;

namespace std
{
	/*template<typename Str, typename CharT>
		requires(!std::same_as<std::remove_cvref_t<std::declval<Str>( )[0]>, CharT>)
	struct formatter<Str, CharT,void> : formatter_froxy<Str, formatter<std::basic_string_view<CharT>, CharT>, convert_adaptor<CharT>>
	{
	};*/

	template<invocable Fn, typename CharT>
	struct formatter<Fn, CharT> : formatter_froxy<Fn, formatter<invoke_result_fmt<Fn>, CharT>, invoke_adaptor<CharT>>
	{
	};
}

//assert if console disabled
void _Log(const std::string_view str) noexcept;
void _Log(const std::wstring_view str) noexcept;

export namespace cheat::console
{
	bool active( ) noexcept;

	void enable( ) noexcept;
	void disable( ) noexcept;

	void log(const std::string_view str) noexcept;
	void log(const std::wstring_view str) noexcept;

	template<std::invocable T>
	void log(T&& fn) noexcept
	{
		if(!active( ))
			return;
		_Log(std::invoke(std::forward<T>(fn)));
	}

	template<typename ...Args>
		requires(sizeof...(Args) > 0)
	void log(const std::wstring_view fmt, Args&& ...args) noexcept
	{
		if(!active( ))
			return;
		_Log(std::vformat(fmt, std::make_wformat_args(_Convert_to<wchar_t>(std::forward<Args>(args))...)));
	}

	template<typename ...Args>
		requires(sizeof...(Args) > 0)
	void log(const std::string_view fmt, Args&& ...args) noexcept
	{
		if(!active( ))
			return;
		_Log(std::vformat(fmt, std::make_format_args(_Convert_to<char>(std::forward<Args>(args))...)));
	}
}