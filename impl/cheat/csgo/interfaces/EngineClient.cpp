module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.EngineClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IVEngineClient, csgo_modules::engine.find_interface<"VEngineClient">( ));