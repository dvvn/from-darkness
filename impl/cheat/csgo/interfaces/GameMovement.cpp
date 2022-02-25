module;

module cheat.csgo.interfaces.GameMovement;
import cheat.csgo.modules;

using namespace cheat::csgo;

CGameMovement* interface_getter<CGameMovement>::set( )const
{
	return csgo_modules::client->find_game_interface("GameMovement");
}