module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.MDLCache;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IMDLCache, csgo_modules::datacache.find_interface<"MDLCache">( ));