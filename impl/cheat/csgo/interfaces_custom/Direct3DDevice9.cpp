#include <cheat/csgo/interface.h>

#include <d3d9.h>

import cheat.csgo.modules;

using namespace cheat;

extern IDirect3DDevice9* d3dDevice9_ptr;

static auto _Find_d3d( ) noexcept
{
	IDirect3DDevice9* ret;
	if(d3dDevice9_ptr)
		ret = csgo_modules::current._Ifc_finder(d3dDevice9_ptr);
	else
		ret = csgo_modules::shaderapidx9.find_interface_sig<"A1 ? ? ? ? 50 8B 08 FF 51 0C">( ).plus(1).deref<2>( );
	return ret;
}

CHEAT_CSGO_INTERFACE_INIT(IDirect3DDevice9, _Find_d3d( ));
