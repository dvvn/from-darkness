module;

#include <fd/object.h>

module fd.valve.input;
import fd.rt_modules;

using namespace fd;
using namespace valve;

FD_OBJECT_IMPL_HEAD(input*)
{
    // fd::rt_modules::client.find_interface_sig<"B9 ? ? ? ? F3 0F 11 04 24 FF 50 10">().plus(1).deref<1>()

    // or address of some indexed input function in chlclient class
    // (like IN_ActivateMouse, IN_DeactivateMouse, IN_Accumulate, IN_ClearStates) + 0x1 (jmp to m_pInput)

    const auto ptr  = rt_modules::client.find_signature<"B9 ? ? ? ? F3 0F 11 04 24 FF 50 10">();
    const auto ptr2 = reinterpret_cast<uintptr_t>(ptr) + 0x1;
    const auto in   = *reinterpret_cast<input**>(ptr2);
    rt_modules::client->log_class_info("class IInput", in);
    _Construct(in);
}

constexpr auto MULTIPLAYER_BACKUP = 150;

user_cmd* input::GetUserCmd(int sequence_number)
{
    return &commands[sequence_number % MULTIPLAYER_BACKUP];
}

verified_user_cmd* input::GetVerifiedCmd(int sequence_number)
{
    return &verified_commands[sequence_number % MULTIPLAYER_BACKUP];
}
