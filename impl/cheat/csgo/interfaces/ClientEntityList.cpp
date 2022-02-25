module cheat.csgo.interfaces.ClientEntityList;
import cheat.csgo.modules;

using namespace cheat::csgo;

IClientEntityList* interface_getter<IClientEntityList>::set( )const
{
	return csgo_modules::client->find_game_interface("VClientEntityList");
}