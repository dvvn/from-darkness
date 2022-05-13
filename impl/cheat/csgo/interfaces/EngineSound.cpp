module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.EngineSound;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IEngineSound, csgo_modules::engine.find_interface<"IEngineSoundClient">( ));