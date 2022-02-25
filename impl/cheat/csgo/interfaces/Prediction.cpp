module cheat.csgo.interfaces.Prediction;
import cheat.csgo.modules;

using namespace cheat::csgo;

IPrediction* interface_getter<IPrediction>::set( )const
{
	return csgo_modules::client->find_game_interface("VClientPrediction");
}