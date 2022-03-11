module;

module cheat.csgo.interfaces.VguiSurface;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

ISurface* nstd::one_instance_getter<ISurface*>::_Construct( )const
{
	return csgo_modules::vguimatsurface->find_game_interface("VGUI_Surface");
}