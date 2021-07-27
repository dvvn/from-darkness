#pragma once

#include "cheat/core/service.h"

namespace cheat::hooks::directx
{
	class present final: public service<present>,
						 public decltype(detect_hook_holder(&IDirect3DDevice9::Present))
	{
	public :
		present( );

	protected:
		bool Do_load( ) override;
		void Callback(THIS_ CONST RECT* source_rect,
					  CONST RECT*       dest_rect,
					  HWND              dest_window_override,
					  CONST RGNDATA*    dirty_region_parameters) override;
	};
}
