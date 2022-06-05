module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.GlobalVars;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(CGlobalVarsBase, csgo_modules::client.find_interface_sig<"A1 ? ? ? ? 5E 8B 40 10">().plus(1).deref<2>());
