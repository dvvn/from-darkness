module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.ModelRender;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IVModelRender, csgo_modules::engine.find_interface<"VEngineModel">());
