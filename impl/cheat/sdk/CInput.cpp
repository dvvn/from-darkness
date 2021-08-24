#include "CInput.hpp"
#include "CUserCmd.hpp"

using namespace cheat::csgo;

CUserCmd* CInput::GetUserCmd(int sequence_number, int nSlot)
{
	return dhooks::_Call_function(&CInput::GetUserCmd, this, 8, nSlot, sequence_number);
}

CVerifiedUserCmd* CInput::GetVerifiedCmd(int sequence_number)
{
	(void)this;
	const auto verified_commands = nstd::address(this).add(0xF8).deref(1).ptr<CVerifiedUserCmd>( );
	return &verified_commands[sequence_number % MULTIPLAYER_BACKUP];
}
