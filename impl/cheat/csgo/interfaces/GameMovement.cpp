module;

module cheat.csgo.interfaces.GameMovement;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CGameMovement* nstd::one_instance_getter<CGameMovement*>::_Construct( )const
{
	return csgo_modules::client.find_interface<"GameMovement">( );
}