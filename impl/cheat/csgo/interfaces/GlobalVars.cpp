module cheat.csgo.interfaces.GlobalVars;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CGlobalVarsBase* nstd::one_instance_getter<CGlobalVarsBase*>::_Construct( )const
{
	return csgo_modules::client->find_signature("A1 ? ? ? ? 5E 8B 40 10").plus(1).deref<2>( );
}