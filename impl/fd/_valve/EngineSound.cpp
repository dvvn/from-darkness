module;

#include <fd/object.h>

module fd.EngineSound;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_IMPL(IEngineSound, fd::rt_modules::engine.find_interface<"IEngineSoundClient">());
