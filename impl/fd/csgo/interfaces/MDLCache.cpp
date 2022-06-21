module;

#include <fd/core/object.h>

module fd.csgo.interfaces.MDLCache;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IMDLCache, 0, runtime_modules::datacache.find_interface<"MDLCache">());
