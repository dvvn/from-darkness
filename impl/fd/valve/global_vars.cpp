module;

#include <fd/object.h>

module fd.valve.global_vars;
import fd.rt_modules;

FD_OBJECT_IMPL(global_vars_base*, 0, fd::runtime_modules::client.find_interface_sig<"A1 ? ? ? ? 5E 8B 40 10">().plus(1).deref<2>());
