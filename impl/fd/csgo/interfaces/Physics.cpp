module;

#include <fd/core/object.h>

module fd.csgo.interfaces.Physics;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IPhysicsSurfaceProps, 0, runtime_modules::vphysics.find_interface<"VPhysicsSurfaceProps">());
