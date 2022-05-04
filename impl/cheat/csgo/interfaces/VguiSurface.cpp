module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.VguiSurface;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(ISurface, csgo_modules::vguimatsurface.find_interface<"VGUI_Surface">( ));