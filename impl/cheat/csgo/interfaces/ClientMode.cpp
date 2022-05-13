module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.ClientMode;
import cheat.csgo.interfaces.BaseClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(ClientModeShared, csgo_modules::client._Ifc_finder(&nstd::instance_of<IBaseClientDLL*>).deref<1>( )[10].plus(5).deref<2>( ));