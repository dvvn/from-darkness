module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.ModelInfoClient;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IVModelInfoClient, csgo_modules::engine.find_interface<"VModelInfoClient">());
