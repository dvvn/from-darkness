module;

#include <fd/object.h>

module fd.valve.local_player;
import fd.rt_modules;

FD_OBJECT_BIND_EX(local_player, fd::rt_modules::client_fn().find_interface_sig<"8B 0D ? ? ? ? 83 FF FF 74 07", 0x2, 1, "LocalPlayer">());
