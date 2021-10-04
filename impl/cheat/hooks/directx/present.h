#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

// ReSharper disable CppInconsistentNaming
struct IDirect3DDevice9;
using HRESULT = long;
struct tagRECT;
struct HWND__;
struct _RGNDATA;
using RECT = tagRECT;
using HWND = HWND__*;
using RGNDATA = _RGNDATA;
// ReSharper restore CppInconsistentNaming

namespace cheat::hooks::directx
{
	class present final: public service_hook_proxy<present, HRESULT(__stdcall IDirect3DDevice9::*)(const RECT*, const RECT*, HWND, const RGNDATA*)>
	{
	public:
		present( );

	protected:
	load_result load_impl() noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void          callback(const RECT*    source_rect,
							   const RECT*    dest_rect,
							   HWND           dest_window_override,
							   const RGNDATA* dirty_region_parameters) override;
	
	};
}
