module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.DebugOverlay;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IVDebugOverlay, csgo_modules::engine.find_interface<"VDebugOverlay">( ));