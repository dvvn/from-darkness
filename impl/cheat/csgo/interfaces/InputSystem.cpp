module;

module cheat.csgo.interfaces.InputSystem;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IInputSystem* nstd::one_instance_getter<IInputSystem*>::_Construct( )const
{
	return csgo_modules::inputsystem.find_interface<"InputSystemVersion">( );
}