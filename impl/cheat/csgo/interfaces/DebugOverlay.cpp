module cheat.csgo.interfaces.DebugOverlay;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IVDebugOverlay* nstd::one_instance_getter<IVDebugOverlay*>::_Construct( )const
{
	return csgo_modules::engine->find_game_interface("VDebugOverlay");
}