module;

#include <fd/core/object.h>

module fd.csgo.interfaces.StudioRender;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IStudioRender, 0, runtime_modules::studiorender.find_interface<"VStudioRender">());
