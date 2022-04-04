module;

module cheat.csgo.interfaces.Direct3DDevice9;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

extern IDirect3DDevice9* d3dDevice9_ptr;

IDirect3DDevice9* nstd::one_instance_getter<IDirect3DDevice9*>::_Construct( ) const
{
	IDirect3DDevice9* ret;

	if (d3dDevice9_ptr)
		ret = d3dDevice9_ptr;
	else
		ret = csgo_modules::shaderapidx9.find_signature<"A1 ? ? ? ? 50 8B 08 FF 51 0C">( ).plus(1).deref<2>( );

	csgo_modules::shaderapidx9.log_found_interface(ret);
	return ret;
}
