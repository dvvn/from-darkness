module;

#include "cheat/hooks/base_includes.h"
#include <d3d9.h>

export module cheat.hooks.directx:present;
import cheat.hooks.base;

namespace cheat::hooks::directx
{
	export struct present final : public hook_base<present, decltype(&IDirect3DDevice9::Present)>
	{
		present( );

	protected:
		void construct( ) noexcept override;
		void callback(const RECT* source_rect,
					  const RECT* dest_rect,
					  HWND dest_window_override,
					  const RGNDATA* dirty_region_parameters) override;
	};

}
