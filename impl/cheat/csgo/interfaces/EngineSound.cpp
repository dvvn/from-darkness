module cheat.csgo.interfaces.EngineSound;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IEngineSound* nstd::one_instance_getter<IEngineSound*>::_Construct( )const
{
	return csgo_modules::engine->find_game_interface("IEngineSoundClient");
}