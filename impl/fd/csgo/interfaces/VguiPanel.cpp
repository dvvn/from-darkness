module;

#include <fd/core/object.h>

module fd.csgo.interfaces.VguiPanel;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IVguiPanel*, 0, runtime_modules::vgui2.find_interface<"VGUI_Panel">());
