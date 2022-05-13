module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.Physics;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IPhysicsSurfaceProps, csgo_modules::vphysics.find_interface<"VPhysicsSurfaceProps">( ));