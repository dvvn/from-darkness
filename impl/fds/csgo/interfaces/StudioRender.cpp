module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.StudioRender;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IStudioRender, csgo_modules::studiorender.find_interface<"VStudioRender">());
