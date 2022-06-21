module;

#include <fd/core/object.h>

module fd.csgo.interfaces.WeaponSystem;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IWeaponSystem, 0, runtime_modules::client.find_interface_sig<"8B 35 ? ? ? ? FF 10 0F B7 C0">().plus(2).deref<1>());
