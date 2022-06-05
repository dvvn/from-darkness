module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.GameMovement;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(CGameMovement, csgo_modules::client.find_interface<"GameMovement">());
