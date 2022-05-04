module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.EngineSound;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IEngineSound, csgo_modules::engine.find_interface<"IEngineSoundClient">( ));