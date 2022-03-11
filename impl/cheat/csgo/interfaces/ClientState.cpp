module;

module cheat.csgo.interfaces.ClientState;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CClientState* nstd::one_instance_getter<CClientState*>::_Construct( )const
{
	return csgo_modules::engine->find_signature("A1 ? ? ? ? 8B 80 ? ? ? ? C3").plus(1).deref<2>( );
}