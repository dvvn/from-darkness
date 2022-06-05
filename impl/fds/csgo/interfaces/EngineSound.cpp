module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.EngineSound;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IEngineSound, csgo_modules::engine.find_interface<"IEngineSoundClient">());
