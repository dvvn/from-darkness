module;

module cheat.csgo.interfaces.GlobalVars;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CGlobalVarsBase* nstd::one_instance_getter<CGlobalVarsBase*>::_Construct( )const
{
	CGlobalVarsBase* const ret = csgo_modules::client.find_signature<"A1 ? ? ? ? 5E 8B 40 10">( ).plus(1).deref<2>( );
	csgo_modules::client.log_found_interface(ret);
	return ret;
}