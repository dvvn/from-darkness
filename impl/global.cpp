#include "cheat/core/console.h"

namespace nstd::detail
{
	void rt_assert_helper(const char* expr, const char* msg, const char* file, const char* func, unsigned int line)
	{
		static auto lock = std::mutex( );
		const auto  _    = std::scoped_lock(lock);

#ifdef _DEBUG
		[[maybe_unused]] const auto from  = _ReturnAddress( );
		[[maybe_unused]] const auto from2 = _AddressOfReturnAddress( );

#endif

		auto message = std::ostringstream( );

		const auto append = [&]<typename Name,typename Value>(Name&& name, Value&& value, bool last = false)
		{
			message << name << ": " << value;
			if (!last)
				message << '\n';
		};

		message << "Assertion falied!\n\n";

		const auto expr_str  = std::string_view(expr);
		const auto skip_expr = [&]
		{
			constexpr auto ignore = std::array{("true"sv), "false"sv, "nullptr"sv};

			if (ranges::find(ignore, expr_str) != ignore.end( ))
				return true;
			if (ranges::all_of(expr_str, static_cast<int(*)(int)>(std::isdigit)))
				return true;

			return false;
		};

		if (!skip_expr( ))
		{
			if (msg != nullptr)
				append("Expression", expr_str);
			else
			{
				message << expr_str << '\n';
			}
		}
		append("File", file);
		append("Line", (line));
		append("Function", func, true);

		if (msg != nullptr)
		{
			message << "\n\n" << msg;
		}

		CHEAT_CONSOLE_LOG(message);
#ifdef _DEBUG
		DebugBreak( );
#endif

		[[maybe_unused]] static volatile auto skip_helper = '\1';
	}
}
