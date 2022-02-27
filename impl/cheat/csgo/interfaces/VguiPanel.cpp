module cheat.csgo.interfaces.VguiPanel;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IVguiPanel* nstd::one_instance_getter<IVguiPanel*>::_Construct( )const
{
	return csgo_modules::vgui2->find_game_interface("VGUI_Panel");
}