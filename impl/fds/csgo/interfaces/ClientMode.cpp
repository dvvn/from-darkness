module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.ClientMode;
import fds.csgo.interfaces.BaseClient;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(ClientModeShared, csgo_modules::client._Ifc_finder(&nstd::instance_of<IBaseClientDLL*>).deref<1>()[10].plus(5).deref<2>());
