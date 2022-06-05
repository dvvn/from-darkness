module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.EngineClient;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IVEngineClient, csgo_modules::engine.find_interface<"VEngineClient">());
