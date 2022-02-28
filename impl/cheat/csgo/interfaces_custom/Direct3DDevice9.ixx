module;

#include <d3d9.h>

export module cheat.csgo.interfaces.Direct3DDevice9;
import nstd.one_instance;

export namespace cheat::csgo
{
	using Direct3DDevice9 = nstd::one_instance<IDirect3DDevice9*>;
}