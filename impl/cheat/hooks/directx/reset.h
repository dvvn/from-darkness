#pragma once

#include "cheat/core/service.h"

namespace cheat::hooks::directx
{
	class reset final: public service<reset>,
					   public decltype(_Detect_hook_holder(&IDirect3DDevice9::Reset))
	{
	public :
		reset( );

	protected:
		bool Do_load( ) override;
		void Callback(D3DPRESENT_PARAMETERS*) override;
	};
}
