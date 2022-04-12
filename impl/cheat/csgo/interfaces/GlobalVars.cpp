module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.GlobalVars;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(CGlobalVarsBase)
{
	CGlobalVarsBase* const ret = csgo_modules::client.find_signature<"A1 ? ? ? ? 5E 8B 40 10">( ).plus(1).deref<2>( );
	csgo_modules::client.log_found_interface(ret);
	return ret;
}