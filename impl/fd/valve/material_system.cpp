module;

#include <fd/object.h>

module fd.valve.material_system;
import fd.rt_modules;

FD_OBJECT_IMPL(material_system, 0, fd::runtime_modules::materialsystem.find_interface<"VMaterialSystem">());
