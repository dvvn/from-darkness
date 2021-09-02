#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

// ReSharper disable CppInconsistentNaming
struct _D3DPRESENT_PARAMETERS_;
using D3DPRESENT_PARAMETERS = _D3DPRESENT_PARAMETERS_;
struct IDirect3DDevice9;
using HRESULT = long;
// ReSharper restore CppInconsistentNaming

namespace cheat::hooks::directx
{
	class reset final: public hook_base<reset, HRESULT(__stdcall IDirect3DDevice9::*)(_D3DPRESENT_PARAMETERS_*)>
	{
	public:
		reset( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(D3DPRESENT_PARAMETERS*) override;
	};
}
