module;

#include "cheat/hooks/base_includes.h"
#include <d3d9.h>

export module cheat.hooks.directx:reset;
import cheat.hooks.base;

namespace cheat::hooks::directx
{
	export class reset final :public hook_base<reset, decltype(&IDirect3DDevice9::Reset)>
	{
	public:
		reset( );

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
		void callback(D3DPRESENT_PARAMETERS*) override;
	};
}
