module;

#include <fd/core/object.h>

module fd.csgo.interfaces.DebugOverlay;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IVDebugOverlay, 0, runtime_modules::engine.find_interface<"VDebugOverlay">());
