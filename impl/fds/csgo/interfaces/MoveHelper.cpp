module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.MoveHelper;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IMoveHelper, csgo_modules::client.find_interface_sig<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01">().plus(2).deref<2>());
