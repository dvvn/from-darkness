module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.ClientState;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(CClientState)
{
	CClientState* const ret = csgo_modules::engine.find_signature<"A1 ? ? ? ? 8B 80 ? ? ? ? C3">( ).plus(1).deref<2>( );
	csgo_modules::client.log_found_interface(ret);
	return ret;
}