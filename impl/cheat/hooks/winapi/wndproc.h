#pragma once
#include "cheat/hooks/base.h"


// ReSharper disable CppInconsistentNaming
using UINT_PTR =
#ifdef _W64
_W64
#endif
unsigned
#if defined(_WIN64)
__int64
#else
int;
#endif
using WPARAM = UINT_PTR;
using LONG_PTR =
#ifdef _W64
_W64
#endif
#if defined(_WIN64)
__int64
#else
long;
#endif
using LPARAM = LONG_PTR;
struct HWND__;
using HWND = HWND__*;
using LRESULT = LONG_PTR;
// ReSharper restore CppInconsistentNaming

namespace cheat::hooks::winapi
{
	using def_wndproc_t = LRESULT (_stdcall*)(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

	struct wndproc_impl final : service<wndproc_impl>, dhooks::_Detect_hook_holder_t<__COUNTER__, def_wndproc_t>
	{
		wndproc_impl( );

	protected:
		load_result load_impl( ) noexcept override;

		void callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

		void* get_target_method( ) const override;

	private:
		def_wndproc_t default_wndproc_ = nullptr;

		bool unicode_ = false;
		HWND hwnd_    = nullptr;

		bool override_return_       = false;
		LRESULT override_return_to_ = 1;
	};

	CHEAT_SERVICE_SHARE(wndproc);
}
