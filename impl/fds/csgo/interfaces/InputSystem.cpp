module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.InputSystem;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IInputSystem, csgo_modules::inputsystem.find_interface<"InputSystemVersion">());
