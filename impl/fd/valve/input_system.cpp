module;

#include <fd/object.h>

module fd.valve.input_system;
import fd.rt_modules;

using namespace fd::valve;

FD_OBJECT_ATTACH_EX(input_system, fd::rt_modules::inputSystem_fn().find_interface<"InputSystemVersion">());
