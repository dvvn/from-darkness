module;

#include <fd/object.h>

module fd.VguiPanel;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_ATTACH_EX(IVguiPanel*, fd::rt_modules::vgui2_fn().find_interface<"VGUI_Panel">());
