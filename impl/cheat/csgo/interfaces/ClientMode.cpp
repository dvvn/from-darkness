module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.ClientMode;
import cheat.csgo.interfaces.BaseClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(ClientModeShared)
{
	return csgo_modules::client._Ifc_finder(IBaseClientDLL::get_ptr( )).deref<1>( )[10].plus(5).deref<2>( );
}