module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.ClientState;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(CClientState, csgo_modules::engine.find_interface_sig<"A1 ? ? ? ? 8B 80 ? ? ? ? C3">( ).plus(1).deref<2>( ));