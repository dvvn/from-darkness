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
	struct present_impl final : service<present_impl>
							  , dhooks::_Detect_hook_holder_t<__COUNTER__, HRESULT(__stdcall IDirect3DDevice9::*)(const RECT*, const RECT*, HWND, const RGNDATA*)>
	{
		present_impl( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback(const RECT* source_rect,
					  const RECT* dest_rect,
					  HWND dest_window_override,
					  const RGNDATA* dirty_region_parameters) override;
	};

	CHEAT_SERVICE_SHARE(present);
}
