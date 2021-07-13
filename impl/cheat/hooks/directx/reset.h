#pragma once

#include "cheat/core/service.h"

namespace cheat::hooks::directx
{
	class reset final: public service_shared<reset, service_mode::async>,
					   public decltype(detect_hook_holder(&IDirect3DDevice9::Reset))
	{
	public :
		reset( );

	protected:
		void Load( ) override;
		void Callback(D3DPRESENT_PARAMETERS*) override;
	};
}
