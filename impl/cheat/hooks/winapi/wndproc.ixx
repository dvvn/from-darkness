module;

#include <windows.h>

export module cheat.hooks.winapi:wndproc;
import dhooks;

namespace cheat::hooks::winapi
{
	using def_wndproc_t = decltype(DefWindowProc)*;
	//using def_wndproc_t = LRESULT(_stdcall*)(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

	export class wndproc final : public dhooks::select_hook_holder<def_wndproc_t>
	{
	public:
		wndproc( );

	protected:
		void callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

	private:
		def_wndproc_t default_wndproc_ = nullptr;

		bool unicode_ = false;
		HWND hwnd_ = nullptr;

		bool override_return_ = false;
		LRESULT override_return_to_ = 1;
	};

}
