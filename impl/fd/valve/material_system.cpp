module;

#include <fd/object.h>

module fd.valve.material_system;
import fd.rt_modules;

FD_OBJECT_IMPL(material_system, fd::rt_modules::materialSystem.find_interface<"VMaterialSystem">());
