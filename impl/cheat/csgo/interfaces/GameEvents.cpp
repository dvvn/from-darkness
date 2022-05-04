module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.GameEvents;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IGameEventManager2, csgo_modules::engine.find_interface<"GAMEEVENTSMANAGER">( ));