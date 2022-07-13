module;

#include <fd/object.h>

module fd.RenderView;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_IMPL(IVRenderView, fd::rt_modules::engine.find_interface<"VEngineRenderView">());
