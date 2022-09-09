module;

#include <fd/object.h>

module fd.valve.input;
import fd.rt_modules;

using namespace fd::valve;

// or address of some indexed input function in chlclient class
// (like IN_ActivateMouse, IN_DeactivateMouse, IN_Accumulate, IN_ClearStates) + 0x1 (jmp to m_pInput)
FD_OBJECT_ATTACH_EX(input*, fd::rt_modules::client_fn().find_interface_sig<"B9 ? ? ? ? F3 0F 11 04 24 FF 50 10", 0x1, 1, "class IInput">());

constexpr auto MULTIPLAYER_BACKUP = 150;

user_cmd* input::GetUserCmd(int sequence_number)
{
    return &commands[sequence_number % MULTIPLAYER_BACKUP];
}

verified_user_cmd* input::GetVerifiedCmd(int sequence_number)
{
    return &verified_commands[sequence_number % MULTIPLAYER_BACKUP];
}
