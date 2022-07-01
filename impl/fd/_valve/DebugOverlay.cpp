module;

#include <fd/object.h>

module fd.DebugOverlay;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_IMPL(IVDebugOverlay, 0, fd::runtime_modules::engine.find_interface<"VDebugOverlay">());
