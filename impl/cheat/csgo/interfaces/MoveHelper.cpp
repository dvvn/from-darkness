module;

module cheat.csgo.interfaces.MoveHelper;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IMoveHelper* nstd::one_instance_getter<IMoveHelper*>::_Construct( )const
{
	return csgo_modules::client.find_signature<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01">( ).plus(2).deref<2>( );
}