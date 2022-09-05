module;

#include <fd/object.h>

module fd.valve.local_player;
import fd.rt_modules;

FD_OBJECT_IMPL_HEAD_BIND(local_player)
{
    _Construct(fd::rt_modules::client.find_interface_sig<"8B 0D ? ? ? ? 83 FF FF 74 07", 0x2, 1, "LocalPlayer">());
}
