module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.ClientState;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(CClientState, csgo_modules::engine.find_interface_sig<"A1 ? ? ? ? 8B 80 ? ? ? ? C3">().plus(1).deref<2>());
