module;

#include <nstd/format.h>
#include <string_view>
//#include <sstream>

export module cheat.console;

export namespace cheat::console
{
	void mark_disabled( );
	bool disabled( );

	void log(const std::string_view str);
	void log(const std::wstring_view str);
	//void log(const std::ostringstream& str);
	//void log(const std::wostringstream& str);

	template<typename Fmt, typename Arg1, typename ...Args>
	void log(Fmt&& fmt, Arg1&& arg1, Args&& ...args)
	{
		if (disabled( ))
			return;

		const auto fmt_fixed = [&]( )->decltype(auto)
		{
#ifdef FMT_VERSION
			using val_t = std::remove_cvref_t<decltype(fmt[0])>;
			//fmt support compile time only for char today
			if constexpr (std::is_same_v<val_t, char>)
				return fmt::runtime(fmt);
			else
#endif
				return std::forward<Fmt>(fmt);
		};

		log(std::format(fmt_fixed( ), std::forward<Arg1>(arg1), std::forward<Args>(args)...));
	}
}