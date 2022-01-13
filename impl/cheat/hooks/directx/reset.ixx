module;

#include "cheat/hooks/base_includes.h"
#include <d3d9.h>

export module cheat.hooks.directx:reset;
import cheat.hooks.base;

export namespace cheat::hooks::directx
{
	struct reset final : hook_base < reset, HRESULT(__stdcall IDirect3DDevice9::*)(_D3DPRESENT_PARAMETERS_*)>
	{
		reset( );

	protected:
		void load_async( ) noexcept override;
		void* get_target_method( ) const override;
		void callback(D3DPRESENT_PARAMETERS*) override;
	};
}
