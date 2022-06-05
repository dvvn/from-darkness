module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.VguiPanel;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IVguiPanel, csgo_modules::vgui2.find_interface<"VGUI_Panel">());
