module;

#include <fd/object.h>

module fd.valve.vgui_surface;
import fd.rt_modules;

FD_OBJECT_ATTACH_EX(vgui_surface*, fd::rt_modules::vguiMatSurface_fn().find_interface<"VGUI_Surface">());
