module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.VguiPanel;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IVguiPanel)
{
	return csgo_modules::vgui2.find_interface<"VGUI_Panel">( );
}