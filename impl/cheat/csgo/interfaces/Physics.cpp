module cheat.csgo.interfaces.Physics;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IPhysicsSurfaceProps* nstd::one_instance_getter<IPhysicsSurfaceProps*>::_Construct( )const
{
	return csgo_modules::vphysics.find_interface<"VPhysicsSurfaceProps">( );
}