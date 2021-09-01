#pragma once
#include "cheat/gui/objects/abstract page.h"
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

	class wndproc final: public base<wndproc, def_wndproc_t>
					   , public gui::objects::empty_page
	{
	public:
		wndproc( );
		void render( ) override;

	protected:
		load_result load_impl( ) override;

		void callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

		nstd::address get_target_method_impl( ) const override;

	private:
		def_wndproc_t default_wndproc_ = nullptr;

		bool unicode_ = false;
		HWND hwnd_    = nullptr;

		bool    override_return_    = false;
		LRESULT override_return_to_ = 1;
	};
}
