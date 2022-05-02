module;

#include <nstd/format.h>

#include <string_view>
#include <functional>
//#include <sstream>

export module cheat.console;

template<typename T>
using invoke_result_fmt = std::remove_cvref_t<decltype(std::invoke(std::declval<T>( )))>;

struct dummy_struct
{
	void func( ) const
	{
	}
};

template<typename T>
constexpr bool is_trivial_object_v = std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(decltype(&dummy_struct::func));

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

struct invoke_adaptor
{
	template<typename ...Ts>
	decltype(auto) operator()(Ts&&...args) const
	{
		return std::invoke(std::forward<Ts>(args)...);
	}
};

namespace std
{
	template<invocable T, typename CharT>
	struct formatter<T, CharT> : formatter_froxy<T, formatter<invoke_result_fmt<T>, CharT>, invoke_adaptor>
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
		requires(sizeof...(Args) >= 2)
	void log(Args&& ...args) noexcept
	{
		if(!active( ))
			return;
		_Log(std::format(std::forward<Args>(args)...));
	}
}