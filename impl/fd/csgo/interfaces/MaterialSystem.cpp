module;

#include <fd/core/object.h>

module fd.csgo.interfaces.MaterialSystem;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IMaterialSystem, 0, runtime_modules::materialsystem.find_interface<"VMaterialSystem">());
