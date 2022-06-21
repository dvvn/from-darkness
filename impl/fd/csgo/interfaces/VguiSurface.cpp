module;

#include <fd/core/object.h>

module fd.csgo.interfaces.VguiSurface;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(ISurface, 0, runtime_modules::vguimatsurface.find_interface<"VGUI_Surface">());
