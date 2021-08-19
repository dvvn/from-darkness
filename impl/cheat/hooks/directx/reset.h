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
		bool         Do_load( ) override;
	utl::address get_target_method_impl( ) const override;
		void         callback(D3DPRESENT_PARAMETERS*) override;
	
	};
}
