module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.MDLCache;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IMDLCache, csgo_modules::datacache.find_interface<"MDLCache">());
