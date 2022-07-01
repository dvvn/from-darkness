module;

#include <fd/object.h>

module fd.StudioRender;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_IMPL(IStudioRender, 0, fd::runtime_modules::studiorender.find_interface<"VStudioRender">());
