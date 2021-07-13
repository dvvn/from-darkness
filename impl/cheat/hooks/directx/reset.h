#pragma once

#include "cheat/core/service.h"

namespace cheat::hooks::directx
{
	class reset final: public service_shared<reset, service_mode::async>,
					   public decltype(detect_hook_holder(&IDirect3DDevice9::Reset)),
					   public service_top_level_only_tag
	{
	public :
		reset( );

	protected:
		auto Load( ) -> void override;
		auto Callback(D3DPRESENT_PARAMETERS*) -> void override;
	};
}
