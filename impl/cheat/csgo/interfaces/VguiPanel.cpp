module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.VguiPanel;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IVguiPanel, csgo_modules::vgui2.find_interface<"VGUI_Panel">( ));