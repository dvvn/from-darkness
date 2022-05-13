module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.InputSystem;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IInputSystem, csgo_modules::inputsystem.find_interface<"InputSystemVersion">( ));