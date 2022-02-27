module cheat.csgo.interfaces.MDLCache;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IMDLCache* nstd::one_instance_getter<IMDLCache*>::_Construct( )const
{
	return csgo_modules::datacache->find_game_interface("MDLCache");
}