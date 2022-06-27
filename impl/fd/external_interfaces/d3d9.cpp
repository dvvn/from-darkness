module;

#include <fd/core/object.h>

module fd.d3d9;
import fd.rt_modules;

FD_OBJECT_BIND(d3d_device9, fd::runtime_modules::shaderapidx9.find_interface_sig<"A1 ? ? ? ? 50 8B 08 FF 51 0C">().plus(1).deref<2>());
