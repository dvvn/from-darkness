module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.DebugOverlay;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IVDebugOverlay, csgo_modules::engine.find_interface<"VDebugOverlay">( ));