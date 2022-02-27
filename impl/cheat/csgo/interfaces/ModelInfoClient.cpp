module cheat.csgo.interfaces.ModelInfoClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IVModelInfoClient* nstd::one_instance_getter<IVModelInfoClient*>::_Construct( )const
{
	return csgo_modules::engine->find_game_interface("VModelInfoClient");
}