module;

#include <fd/object.h>

module fd.valve.mdl_cache;
import fd.rt_modules;

FD_OBJECT_IMPL(mdl_cache, 0, fd::runtime_modules::datacache.find_interface<"mdl_cache">());