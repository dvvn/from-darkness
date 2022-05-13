module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.ModelInfoClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IVModelInfoClient, csgo_modules::engine.find_interface<"VModelInfoClient">( ));