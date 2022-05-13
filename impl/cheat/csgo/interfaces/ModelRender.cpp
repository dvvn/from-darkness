module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.ModelRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IVModelRender, csgo_modules::engine.find_interface<"VEngineModel">( ));