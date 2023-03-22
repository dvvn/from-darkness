module;

#include <fd/object.h>

module fd.EngineSound;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_ATTACH_EX(IEngineSound, fd::rt_modules::engine_fn(.find_interface<"IEngineSoundClient">());
