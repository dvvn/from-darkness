module;

#include <fd/core/object.h>

module fd.csgo.interfaces.EngineClient;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IVEngineClient, 0, runtime_modules::engine.find_interface<"VEngineClient">());
