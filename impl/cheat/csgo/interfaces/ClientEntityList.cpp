module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.ClientEntityList;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IClientEntityList, csgo_modules::client.find_interface<"VClientEntityList">( ));