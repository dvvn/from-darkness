#include "IBaseClientDll.hpp"

#include <dhooks/helpers.h>

using namespace cheat::csgo;

bool IBaseClientDLL::DispatchUserMessage(int msg_type, int32_t flags, int size, const void* msg)
{
	return dhooks::_Call_function(&IBaseClientDLL::DispatchUserMessage, this, 38, msg_type, flags, size, msg);
}
