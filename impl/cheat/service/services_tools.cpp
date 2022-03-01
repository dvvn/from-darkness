module;

#include <nstd/format.h>

module cheat.service:tools;
import cheat.console;
import cheat.root_service;

using namespace cheat;

void cheat::log_service_start(basic_service* holder, basic_service::state_type result)
{
	console::log("{} - {}.", holder->name( ), result == basic_service::state_type::loaded ? "Loaded successfully" : "Not loaded");
}