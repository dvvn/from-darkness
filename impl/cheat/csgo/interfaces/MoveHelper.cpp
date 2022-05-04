module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.MoveHelper;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IMoveHelper, csgo_modules::client.find_interface_sig<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01">( ).plus(2).deref<2>( ));