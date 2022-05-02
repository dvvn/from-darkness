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

#if 1
#define LOG_STR(_TYPE_) \
case Log::LT_##_TYPE_:  \
	str = #_TYPE_;		\
	break;

template<>
struct std::formatter<Log::Type, char> : formatter<std::string_view>
{
	template<class FormatContext>
	auto format(const Log::Type type, FormatContext& fc) const
	{
		std::string_view str;
		switch(type)
		{
			LOG_STR(ALWAYS);
			LOG_STR(ERROR);
			LOG_STR(ASSERT);
			LOG_STR(WARNING);
			LOG_STR(INFO);
			LOG_STR(DEBUG);
			default:
				runtime_assert("Unknown log type detected");
		}

		return formatter<std::string_view>::format(str, fc);
	}
};

#endif

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
			console::log("[RmlUi][ERROR] {}", message);
			return false;
		case Log::LT_ALWAYS:
			if(!console::active( ))
				return SystemInterface::LogMessage(logtype, message);
			console::log("[RmlUi] {}", message);
			return true;

		case Log::LT_DEBUG:
#ifndef _DEBUG
			return true;
#endif
		case Log::LT_WARNING:
		case Log::LT_INFO:
			console::log("[RmlUi][{}] {}", logtype, message);
			return true;
		default:
			runtime_assert("Unknown log type detected");
			return false;

	}
}