module;

#include <fd/object.h>

module fd.ModelRender;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_IMPL(IVModelRender, fd::rt_modules::engine.find_interface<"VEngineModel">());
