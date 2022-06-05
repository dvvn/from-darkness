module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.DebugOverlay;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IVDebugOverlay, csgo_modules::engine.find_interface<"VDebugOverlay">());
