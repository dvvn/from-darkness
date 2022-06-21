module;

#include <fd/core/object.h>

module fd.csgo.interfaces.Input;
import fd.csgo.structs.UserCmd;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(CInput*, 0, runtime_modules::client.find_interface_sig<"B9 ? ? ? ? F3 0F 11 04 24 FF 50 10">().plus(1).deref<1>());

constexpr auto MULTIPLAYER_BACKUP = 150;

CUserCmd* CInput::GetUserCmd(int sequence_number)
{
    return &pCommands[sequence_number % MULTIPLAYER_BACKUP];
}

CVerifiedUserCmd* CInput::GetVerifiedCmd(int sequence_number)
{
    return &pVerifiedCommands[sequence_number % MULTIPLAYER_BACKUP];
}
