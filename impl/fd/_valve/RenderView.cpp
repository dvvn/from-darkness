module;

#include <fd/object.h>

module fd.RenderView;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_ATTACH_EX(IVRenderView, fd::rt_modules::engine_fn().find_interface<"VEngineRenderView">());
