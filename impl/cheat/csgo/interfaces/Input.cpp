module;

#include <dhooks/helpers.h>

module cheat.csgo.interfaces:Input;
import :UserCmd;
import nstd.address;

using namespace cheat::csgo;

CUserCmd* CInput::GetUserCmd(int sequence_number, int nSlot)
{
	return dhooks::_Call_function(&CInput::GetUserCmd, this, 8, nSlot, sequence_number);
}
CVerifiedUserCmd* CInput::GetVerifiedCmd(int sequence_number)
{
	constexpr auto MULTIPLAYER_BACKUP = 150;
	CVerifiedUserCmd* verified_commands = nstd::address(this).add(0xF8).deref(1).ptr( );
	return &verified_commands[sequence_number % MULTIPLAYER_BACKUP];
}
