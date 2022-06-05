module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.VguiSurface;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(ISurface, csgo_modules::vguimatsurface.find_interface<"VGUI_Surface">());
