module;

#include <fd/object.h>

module fd.valve.vgui_surface;
import fd.rt_modules;

FD_OBJECT_IMPL(vgui_surface*, fd::rt_modules::vguimatsurface.find_interface<"VGUI_Surface">());
