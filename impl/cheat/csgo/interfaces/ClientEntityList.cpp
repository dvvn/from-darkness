module cheat.csgo.interfaces.ClientEntityList;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IClientEntityList* nstd::one_instance_getter<IClientEntityList*>::_Construct( ) const
{
	return csgo_modules::client->find_game_interface("VClientEntityList");
}