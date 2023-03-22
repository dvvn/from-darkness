module;

#include <fd/object.h>

module fd.StudioRender;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_ATTACH_EX(IStudioRender, fd::rt_modules::studiorender_fn().find_interface<"VStudioRender">());
