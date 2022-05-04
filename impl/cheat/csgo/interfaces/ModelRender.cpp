module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.ModelRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IVModelRender, csgo_modules::engine.find_interface<"VEngineModel">( ));