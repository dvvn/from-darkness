module;

#include <d3d9.h>

export module cheat.hooks.directx:reset;
import dhooks;

namespace cheat::hooks::directx
{
	export class reset final :public dhooks::select_hook_holder<decltype(&IDirect3DDevice9::Reset)>
	{
	public:
		reset( );

	protected:
		void callback(D3DPRESENT_PARAMETERS*) override;
	};
}
