#include "IBaseClientDll.hpp"

using namespace cheat::csgo;

void IBaseClientDLL::FrameStageNotify(ClientFrameStage_t stage)
{
	BOOST_ASSERT("Dont use. Added only for example.");
	(void)this;
}

bool IBaseClientDLL::DispatchUserMessage(int msg_type, int32_t flags, int size, const void* msg)
{
	return utl::hooks::call_virtual_class_method(&IBaseClientDLL::DispatchUserMessage, this, 38, msg_type, flags, size, msg);
}
