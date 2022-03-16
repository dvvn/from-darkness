module;

module cheat.csgo.interfaces.GameEvents;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IGameEventManager2* nstd::one_instance_getter<IGameEventManager2*>::_Construct( )const
{
	return csgo_modules::engine.find_interface<"GAMEEVENTSMANAGER">( );
}