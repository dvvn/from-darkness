module;

#include <fd/object.h>

module fd.valve.local_player;
import fd.rt_modules;

FD_OBJECT_BIND(local_player, fd::runtime_modules::client.find_interface_sig<"8B 0D ? ? ? ? 83 FF FF 74 07">().plus(2).deref<1>());
