#pragma once

#include "cheat/core/service.h"

namespace cheat::hooks::directx
{
	class present final: public service_shared<present, service_mode::async>,
						 public decltype(detect_hook_holder(&IDirect3DDevice9::Present))
	{
	public :
		present( );

	protected:
		auto Load( ) -> void override;
		auto Callback(THIS_ CONST RECT* source_rect,
					  CONST RECT*       dest_rect,
					  HWND              dest_window_override,
					  CONST RGNDATA*    dirty_region_parameters) -> void override;
	};
}
