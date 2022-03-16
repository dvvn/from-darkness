module;

module cheat.csgo.interfaces.Prediction;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IPrediction* nstd::one_instance_getter<IPrediction*>::_Construct( )const
{
	return csgo_modules::client.find_interface<"VClientPrediction">( );
}