module;

#include <fd/object.h>

module fd.VguiPanel;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_IMPL(IVguiPanel*, 0, fd::runtime_modules::vgui2.find_interface<"VGUI_Panel">());
