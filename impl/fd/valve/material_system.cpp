module;

#include <fd/object.h>

module fd.valve.material_system;
import fd.rt_modules;

FD_OBJECT_ATTACH_EX(material_system, fd::rt_modules::materialSystem_fn().find_interface<"VMaterialSystem">());
