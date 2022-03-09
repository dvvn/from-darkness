module;

#include <d3d9.h>

export module cheat.hooks.directx:present;
import dhooks;

namespace cheat::hooks::directx
{
	export class present final : public dhooks::select_hook_holder<decltype(&IDirect3DDevice9::Present)>
	{
	public:
		present( );

	protected:
		void callback(const RECT* source_rect,
					  const RECT* dest_rect,
					  HWND dest_window_override,
					  const RGNDATA* dirty_region_parameters) override;
	};
}
