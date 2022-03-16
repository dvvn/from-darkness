module;

module cheat.csgo.interfaces.MaterialSystem;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IMaterialSystem* nstd::one_instance_getter<IMaterialSystem*>::_Construct( )const
{
	return csgo_modules::materialsystem.find_interface<"VMaterialSystem">( );
}