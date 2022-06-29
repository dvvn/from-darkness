module;

#include <fd/object.h>

module fd.valve.vgui_surface;
import fd.rt_modules;

FD_OBJECT_IMPL(vgui_surface*, 0, fd::runtime_modules::vguimatsurface.find_interface<"VGUI_Surface">());
