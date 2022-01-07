module;

#include <functional>

module cheat.csgo.interfaces:BaseClient;
import dhooks;

using namespace cheat::csgo;

bool IBaseClientDLL::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
	return dhooks::call_function(&IBaseClientDLL::DispatchUserMessage, this, 38, msg_type, flags, size, msg);
}
