#include "CInput.hpp"
#include "CUserCmd.hpp"

#include "cheat/utils/hooks/hook_utils.h"

using namespace cheat::csgo;

CUserCmd* CInput::GetUserCmd(int sequence_number, int nSlot)
{
	return utl::hooks::call_virtual_class_method(&CInput::GetUserCmd, this, 8, nSlot, sequence_number);
}

CVerifiedUserCmd* CInput::GetVerifiedCmd(int sequence_number)
{
	auto verified_commands = utl::mem::address(this).add(0xF8).deref(1).raw<CVerifiedUserCmd>( );
	return &verified_commands[sequence_number % MULTIPLAYER_BACKUP];
}
