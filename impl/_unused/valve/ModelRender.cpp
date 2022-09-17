module;

#include <fd/object.h>

module fd.ModelRender;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_ATTACH_EX(IVModelRender, fd::rt_modules::engine_fn().find_interface<"VEngineModel">());
