module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.GlobalVars;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(CGlobalVarsBase, csgo_modules::client.find_interface_sig<"A1 ? ? ? ? 5E 8B 40 10">( ).plus(1).deref<2>( ));