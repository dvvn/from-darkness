module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.Physics;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IPhysicsSurfaceProps)
{
	return csgo_modules::vphysics.find_interface<"VPhysicsSurfaceProps">( );
}