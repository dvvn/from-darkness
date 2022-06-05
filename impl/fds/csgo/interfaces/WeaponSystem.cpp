module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.WeaponSystem;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IWeaponSystem, csgo_modules::client.find_interface_sig<"8B 35 ? ? ? ? FF 10 0F B7 C0">().plus(2).deref<1>());
