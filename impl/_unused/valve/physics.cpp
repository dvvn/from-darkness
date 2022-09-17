module;

#include <fd/object.h>

module fd.valve.physics;
import fd.rt_modules;

FD_OBJECT_ATTACH_EX(physics_surface_props, fd::rt_modules::vphysics_fn().find_interface<"VPhysicsSurfaceProps">());
