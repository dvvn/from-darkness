module;

#include <fd/core/object.h>

module fd.csgo.interfaces.InputSystem;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IInputSystem, 0, runtime_modules::inputsystem.find_interface<"InputSystemVersion">());
