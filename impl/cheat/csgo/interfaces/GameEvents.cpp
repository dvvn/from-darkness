module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.GameEvents;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IGameEventManager2, csgo_modules::engine.find_interface<"GAMEEVENTSMANAGER">( ));