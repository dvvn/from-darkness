module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.Physics;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IPhysicsSurfaceProps, csgo_modules::vphysics.find_interface<"VPhysicsSurfaceProps">());
