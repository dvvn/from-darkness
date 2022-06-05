module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.GameEvents;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IGameEventManager2, csgo_modules::engine.find_interface<"GAMEEVENTSMANAGER">());
