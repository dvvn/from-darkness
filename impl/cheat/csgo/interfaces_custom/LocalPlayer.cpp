module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.LocalPlayer;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(C_CSPlayer*, csgo_modules::client.find_interface_sig<"8B 0D ? ? ? ? 83 FF FF 74 07">( ).plus(2).deref<1>( ));
