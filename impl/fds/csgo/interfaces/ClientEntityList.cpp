module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.ClientEntityList;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IClientEntityList, csgo_modules::client.find_interface<"VClientEntityList">());
