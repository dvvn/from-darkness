module;

#include <cheat/service/basic_includes.h>

export module cheat.service:tools;
export import :basic;

export namespace cheat
{
	//void log_service_state(basic_service* holder, service_state state);

	void log_service_start(basic_service* holder, basic_service::state_type result);
}