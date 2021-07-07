#include "wndproc.h"

#include "cheat/core/root service.h"
#include "cheat/gui/user input.h"

using namespace cheat;
using namespace hooks;
using namespace gui;
using namespace input;
using namespace utl;

wndproc::wndproc( )
{
	this->Wait_for<user_input>( );
}

void wndproc::Load( )
{
	auto hwnd = user_input::get( ).hwnd( );

	const bool unicode = IsWindowUnicode(hwnd);
	const auto game_wndproc = reinterpret_cast<WNDPROC>(invoke(unicode ? GetWindowLongPtrW : GetWindowLongPtrA, hwnd, GWLP_WNDPROC));

	default_wndproc__ = unicode ? DefWindowProcW : DefWindowProcA;
	target_func_ = method_info::make_static(game_wndproc);

	this->hook( );
	this->enable( );
}

void wndproc::Callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef CHEAT_GUI_TEST
	if (wparam == VK_DELETE && msg == WM_KEYUP)
	{
		this->disable( );
		this->return_value_.store_value(TRUE);
		root_service::get( ).unload( );
		return;
	}
#endif

	using result = user_input::process_result;

	switch (user_input::get( ).process(hwnd, msg, wparam, lparam))
	{
		case result::blocked:
		{
			this->return_value_.store_value(TRUE);
			break;
		}
		case result::skipped:
		{
			this->return_value_.store_value(default_wndproc__(hwnd, msg, wparam, lparam));
			break;
		}
	}
}
