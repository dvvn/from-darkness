module;

#include "cheat/hooks/base_includes.h"
#include <d3d9.h>

export module cheat.hooks.directx:present;
import cheat.hooks.base;

export namespace cheat::hooks::directx
{
	struct present final : hook_base<present
		, HRESULT(__stdcall IDirect3DDevice9::*)(const RECT*, const RECT*, HWND, const RGNDATA*)>
	{
		present( );

	protected:
		void construct( ) noexcept override;
		void* get_target_method( ) const override;
		void callback(const RECT* source_rect,
					  const RECT* dest_rect,
					  HWND dest_window_override,
					  const RGNDATA* dirty_region_parameters) override;
	};

}
