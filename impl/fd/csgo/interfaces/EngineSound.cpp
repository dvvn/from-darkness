module;

#include <fd/core/object.h>

module fd.csgo.interfaces.EngineSound;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IEngineSound, 0, runtime_modules::engine.find_interface<"IEngineSoundClient">());
