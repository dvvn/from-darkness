module;

module cheat.csgo.interfaces.EngineClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IVEngineClient* nstd::one_instance_getter<IVEngineClient*>::_Construct( )const
{
	return csgo_modules::engine.find_interface<"VEngineClient">( );
}