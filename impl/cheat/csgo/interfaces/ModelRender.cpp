module;

module cheat.csgo.interfaces.ModelRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IVModelRender* nstd::one_instance_getter<IVModelRender*>::_Construct( )const
{
	return csgo_modules::engine.find_interface<"VEngineModel">( );
}