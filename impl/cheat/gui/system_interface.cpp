module;

#include <nstd/runtime_assert.h>
#include <nstd/format.h>

#include <RmlUi/Core.h>

#include <chrono>

module cheat.gui.system_interface;
import cheat.console;

using namespace cheat;
using namespace gui;

system_interface::system_interface( )
	:start_time_(clock::now( ))
{
}

double system_interface::GetElapsedTime( )
{
	return duration_cast<out_duration>(clock::now( ) - start_time_).count( );
}

using namespace Rml;

template<>
struct std::formatter<Log::Type, char> : formatter<std::string_view>
{
	template<class FormatContext>
	auto format(const Log::Type type, FormatContext& fc) const
	{
		std::string_view str;
		switch(type)
		{
			case Log::LT_ERROR:
				str = "ERROR";
				break;
			case Log::LT_ASSERT:
				str = "FATAL ERROR";
				break;
			case Log::LT_WARNING:
				str = "WARNING";
				break;
			case Log::LT_ALWAYS:
			case Log::LT_INFO:
				str = "INFO";
				break;
			case Log::LT_DEBUG:
				str = "DEBUG";
				break;
			default:
				runtime_assert("Unknown log type detected");
				str = "UNKNOWN";
				break;
		}

		return formatter<std::string_view>::format(str, fc);
	}
};

template<typename ...Args>
static void _Log(const std::string_view str, Args&& ...args) noexcept
{
	console::log("[RmlUi] {}", [&]
	{
		return std::vformat(str, std::make_format_args(std::forward<Args>(args)...));
	});
}

static void _Log(const Log::Type logtype, const String& message)
{
	_Log("{}! {}", logtype, message);
}

bool system_interface::LogMessage(Log::Type logtype, const String& message)
{
	switch(logtype)
	{
		case Log::LT_ASSERT:
		{
			runtime_assert(message.c_str( ));//it calls std::terminate if defined
			return SystemInterface::LogMessage(logtype, message);//otherwise call default logs handler
		}
		case Log::LT_ERROR:
			_Log(logtype, message);
			return false;
		case Log::LT_DEBUG:
#ifndef _DEBUG
			return true;
#endif
		case Log::LT_ALWAYS:
			if(!console::active( ))
				return SystemInterface::LogMessage(logtype, message);
		case Log::LT_WARNING:
		case Log::LT_INFO:
			_Log(logtype, message);
			return true;
		default:
			runtime_assert("Unknown log type detected");
			return false;
	}
}