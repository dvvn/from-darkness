module;

#include <nstd/format.h>
#include <string_view>
#include <functional>
//#include <sstream>

export module cheat.console;

template<typename F>
decltype(auto) fmt_fix(F&& fmt)
{
#ifdef FMT_VERSION
	using val_t = std::remove_cvref_t<decltype(fmt[0])>;
	//fmt support compile time only for char today
	if constexpr (std::is_same_v<val_t, char>)
		return fmt::runtime(fmt);
	else
#endif
		return std::forward<F>(fmt);
}

template<typename F, typename ...Args>
auto format_impl(F&& fmt, Args&& ...args)
{
	return std::format(fmt_fix(std::forward<F>(fmt)), std::forward<Args>(args)...);
}

template<typename T, typename Chr = char>
concept formattable1 = requires
{
	{ std::formatter<T, Chr> }->std::default_initializable;
};

template<typename Chr, typename T>
decltype(auto) unpack_invocable(T&& obj)
{
	using raw_t = std::remove_cvref_t<T>;

	if constexpr (std::invocable<raw_t> && !formattable1<raw_t, Chr>)
		return unpack_invocable<Chr>(std::invoke(std::forward<T>(obj)));
	else
		return std::forward<T>(obj);
}

template<typename Arg, typename ...Args>
using char_t = std::remove_cvref_t<decltype(std::declval<Arg>( )[0])>;

bool active( );

export namespace cheat::console
{
	void enable( );
	void disable( );

	void log(const std::string_view str);
	void log(const std::wstring_view str);
	//void log(const std::ostringstream& str);
	//void log(const std::wostringstream& str);

	template<std::invocable T>
	void log(T fn)
	{
		if (!active( ))
			return;
		log(std::invoke(fn));
	}

	template<typename ...Args>
		requires(sizeof...(Args) >= 2)
	void log(Args&& ...args)
	{
		if (!active( ))
			return;
		using chr = char_t<Args...>;
		log(format_impl(unpack_invocable<chr>(std::forward<Args>(args))...));
	}
}