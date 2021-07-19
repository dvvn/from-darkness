#if defined(BOOST_ENABLE_ASSERT_HANDLER) || (defined(BOOST_ENABLE_ASSERT_DEBUG_HANDLER) && !defined(NDEBUG) )

#ifdef CHEAT_DEBUG_MODE

#include "cheat/core/console.h"

void _Console_log(const std::string_view& str)
{
	const auto ptr = cheat::console::get_shared( );
	if (ptr == nullptr || !ptr->initialized( ))
		return;
	constexpr auto filler = std::string_view("\n--------------------------------------------------\n");
	std::string    str2;
	str2.reserve(filler.size( ) * 2 + str.size( ));
	str2 += filler;
	str2 += str;
	str2 += filler;
	ptr->write_line(str2);
}
#else

#define _Console_log(x)

#endif
namespace boost
{
	// ReSharper disable All

	static bool ignore_error = false;

	void assertion_failed(char const* expr, char const* function, char const* file, long line)
	{
		DebugBreak( );
		if (ignore_error)
		{
			ignore_error = 0;
			return;
		}
		auto str = cheat::utl::format("Assertion falied!\nExpression: {}\n\nFile: {}\nLine: {}\nFunction: {}", expr, file, line, function);
		_Console_log(str);
		throw std::runtime_error(str);
	}

	void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line)
	{
		DebugBreak( );
		if (ignore_error)
		{
			ignore_error = 0;
			return;
		}
		auto str = cheat::utl::format("Assertion falied!\nExpression: {}\nMessage:{}\n\nFile: {}\nLine: {}\nFunction: {}", expr, msg, file, line, function);
		_Console_log(str);
		throw std::runtime_error(str);
	}
}

#endif
