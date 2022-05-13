module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.EngineClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IVEngineClient, csgo_modules::engine.find_interface<"VEngineClient">( ));