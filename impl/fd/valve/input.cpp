module;

#include <fd/object.h>

module fd.valve.input;
import fd.rt_modules;

// FD_OBJECT_IMPL(input*, fd::rt_modules::client.find_interface_sig<"B9 ? ? ? ? F3 0F 11 04 24 FF 50 10">().plus(1).deref<1>());

constexpr auto MULTIPLAYER_BACKUP = 150;

using namespace fd::valve;

user_cmd* input::GetUserCmd(int sequence_number)
{
    return &commands[sequence_number % MULTIPLAYER_BACKUP];
}

verified_user_cmd* input::GetVerifiedCmd(int sequence_number)
{
    return &verified_commands[sequence_number % MULTIPLAYER_BACKUP];
}
