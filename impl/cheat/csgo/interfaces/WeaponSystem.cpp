module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.WeaponSystem;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IWeaponSystem, csgo_modules::client.find_interface_sig<"8B 35 ? ? ? ? FF 10 0F B7 C0">( ).plus(2).deref<1>( ));