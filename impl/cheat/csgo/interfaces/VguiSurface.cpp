module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.VguiSurface;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(ISurface, csgo_modules::vguimatsurface.find_interface<"VGUI_Surface">( ));