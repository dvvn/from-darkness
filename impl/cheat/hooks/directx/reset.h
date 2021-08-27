#pragma once

#include "cheat/core/service.h"

namespace cheat::hooks::directx
{
	class reset final: public service<reset>
					 , public dhooks::_Detect_hook_holder_t<decltype(&IDirect3DDevice9::Reset)>
					 , service_hook_helper
	{
	public:
		reset( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(D3DPRESENT_PARAMETERS*) override;
	};
}
