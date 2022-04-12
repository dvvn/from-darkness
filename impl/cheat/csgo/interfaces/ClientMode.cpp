module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.ClientMode;
import cheat.csgo.interfaces.BaseClient;
import cheat.csgo.modules;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(ClientModeShared)
{
	const nstd::basic_address addr = IBaseClientDLL::get_ptr( );
	ClientModeShared* const ret = addr.deref<1>( )[10].plus(5).deref<2>( );
	csgo_modules::client.log_found_interface(ret);
	return ret;
}