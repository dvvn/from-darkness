#ifdef BOOST_ENABLE_ASSERT_HANDLER //|| (defined(BOOST_ENABLE_ASSERT_DEBUG_HANDLER) && !defined(NDEBUG) )

#include "cheat/core/console.h"

[[maybe_unused]] static void _Console_log(const std::string_view& str)
{
	const auto wptr = cheat::console::get_ptr_weak( );
	if (wptr.expired( ))
		return;
	const auto sptr = wptr.lock( );
	const auto ptr = sptr.get( );
	if (!ptr->state( ).done())
		return;
	constexpr auto filler = std::string_view("\n--------------------------------------------------\n");
	std::string str2;
	str2.reserve(filler.size( ) * 2 + str.size( ));
	str2 += filler;
	str2 += str;
	str2 += filler;
	ptr->write_line(str2);
}

namespace boost
{
	using namespace cheat::utl;

	static volatile auto ignore_error = false;

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
#ifdef CHEAT_HAVE_CONSOLE
		_Console_log(str);
#endif
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
#ifdef CHEAT_HAVE_CONSOLE
		_Console_log(str);
#endif
		throw std::runtime_error(str);
	}

	// ReSharper restore CppEnforceCVQualifiersPlacement
}
#endif
