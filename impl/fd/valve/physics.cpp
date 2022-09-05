module;

#include <fd/object.h>

module fd.valve.physics;
import fd.rt_modules;

FD_OBJECT_IMPL(physics_surface_props, fd::rt_modules::vphysics.find_interface<"VPhysicsSurfaceProps">());
