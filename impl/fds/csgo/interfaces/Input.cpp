module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.Input;
import fds.csgo.structs.UserCmd;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(CInput, csgo_modules::client.find_interface_sig<"B9 ? ? ? ? F3 0F 11 04 24 FF 50 10">().plus(1).deref<1>());

constexpr auto MULTIPLAYER_BACKUP = 150;

CUserCmd* CInput::GetUserCmd(int sequence_number)
{
    return &pCommands[sequence_number % MULTIPLAYER_BACKUP];
}

CVerifiedUserCmd* CInput::GetVerifiedCmd(int sequence_number)
{
    return &pVerifiedCommands[sequence_number % MULTIPLAYER_BACKUP];
}
