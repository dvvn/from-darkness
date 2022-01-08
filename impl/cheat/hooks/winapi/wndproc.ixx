module;

#include "cheat/hooks/base_includes.h"
#include <windows.h>

export module cheat.hooks.winapi:wndproc;
import cheat.hooks.base;

namespace cheat::hooks::winapi
{
	using def_wndproc_t = decltype(DefWindowProc)*;
	//using def_wndproc_t = LRESULT(_stdcall*)(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

	export struct wndproc final : hook_base<wndproc, def_wndproc_t>
	{
		wndproc( );

	protected:
		void load_async( ) noexcept override;
		bool load_impl( ) noexcept override;
		void callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;
		void* get_target_method( ) const override;

	private:
		def_wndproc_t default_wndproc_ = nullptr;

		bool unicode_ = false;
		HWND hwnd_ = nullptr;

		bool override_return_ = false;
		LRESULT override_return_to_ = 1;
	};

	//CHEAT_SERVICE_SHARE(wndproc);
}
