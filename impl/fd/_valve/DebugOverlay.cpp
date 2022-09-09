module;

#include <fd/object.h>

module fd.DebugOverlay;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_ATTACH_EX(IVDebugOverlay, fd::rt_modules::engine_fn(.find_interface<"VDebugOverlay">());
