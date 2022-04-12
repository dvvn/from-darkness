module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.InputSystem;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IInputSystem)
{
	return csgo_modules::inputsystem.find_interface<"InputSystemVersion">( );
}