module;

#include <fd/object.h>

module fd.valve.mdl_cache;
import fd.rt_modules;

FD_OBJECT_ATTACH_EX(mdl_cache, fd::rt_modules::dataCache_fn().find_interface<"MdlCache">());
