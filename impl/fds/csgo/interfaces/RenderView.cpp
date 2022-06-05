module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.RenderView;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IVRenderView, csgo_modules::engine.find_interface<"VEngineRenderView">());
