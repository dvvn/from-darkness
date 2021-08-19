#pragma once

#include "cheat/core/service.h"

namespace cheat::hooks::directx
{
	class present final: public service<present>,
						 public decltype(_Detect_hook_holder(&IDirect3DDevice9::Present))
	{
	public :
		present( );

	protected:
		bool         Do_load( ) override;
		utl::address get_target_method_impl( ) const override;
		void         callback(THIS_ CONST RECT* source_rect,
							  CONST RECT*       dest_rect,
							  HWND              dest_window_override,
							  CONST RGNDATA*    dirty_region_parameters) override;
	};
}
