module;

#include <fd/object.h>

module fd.valve.weapon_system;
import fd.rt_modules;

FD_OBJECT_IMPL(weapon_system, (fd::rt_modules::client.find_interface_sig<"8B 35 ? ? ? ? FF 10 0F B7 C0", 0x2, 1, "class IWeaponSystem">()));
