module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.GameMovement;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(CGameMovement, csgo_modules::client.find_interface<"GameMovement">( ));