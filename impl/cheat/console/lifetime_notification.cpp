module;

#include <nstd/format.h>

module cheat.console.lifetime_notification;
import cheat.console;

void console_log(const std::string_view name, const std::string_view msg)
{
	cheat::console::log("{}: {}", name, msg);
}
