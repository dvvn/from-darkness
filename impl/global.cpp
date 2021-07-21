#if defined(BOOST_ENABLE_ASSERT_HANDLER) || (defined(BOOST_ENABLE_ASSERT_DEBUG_HANDLER) && !defined(NDEBUG) )

#ifdef CHEAT_HAVE_CONSOLE
#include "cheat/core/console.h"
#endif

static void _Console_log(const std::string_view& str)
{
#ifdef CHEAT_HAVE_CONSOLE
	const auto wptr = cheat::console::get_weak( );
	if (wptr.expired( ))
		return;
	const auto sptr = wptr.lock( );
	const auto ptr = sptr.get( );
	if (!ptr->initialized( ))
		return;
	constexpr auto filler = std::string_view("\n--------------------------------------------------\n");
	std::string str2;
	str2.reserve(filler.size( ) * 2 + str.size( ));
	str2 += filler;
	str2 += str;
	str2 += filler;
	ptr->write_line(str2);
#endif
}

namespace boost
{
	using namespace cheat::utl;

	static auto ignore_error = false;

	// ReSharper disable CppEnforceCVQualifiersPlacement

	void assertion_failed(char const* expr, char const* function, char const* file, long line)
	{
		DebugBreak( );
		if (ignore_error)
		{
			ignore_error = false;
			return;
		}
		const auto str = format("Assertion falied!\nExpression: {}\n\nFile: {}\nLine: {}\nFunction: {}", expr, file, line, function);
		_Console_log(str);
		throw std::runtime_error(str);
	}

	void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line)
	{
		DebugBreak( );
		if (ignore_error)
		{
			ignore_error = false;
			return;
		}
		const auto str = format("Assertion falied!\nExpression: {}\nMessage:{}\n\nFile: {}\nLine: {}\nFunction: {}", expr, msg, file, line, function);
		_Console_log(str);
		throw std::runtime_error(str);
	}

	// ReSharper restore CppEnforceCVQualifiersPlacement
}
#endif
