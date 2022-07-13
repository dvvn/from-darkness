module;

#include <fd/object.h>

module fd.valve.input_system;
import fd.rt_modules;

using namespace fd::valve;

FD_OBJECT_IMPL(input_system, fd::rt_modules::inputsystem.find_interface<"InputSystemVersion">());
