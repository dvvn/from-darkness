module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.GameMovement;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(CGameMovement, csgo_modules::client.find_interface<"GameMovement">( ));