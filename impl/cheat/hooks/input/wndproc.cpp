#include "wndproc.h"

#include "cheat/core/services loader.h"
#include "cheat/gui/user input.h"
#include "cheat/gui/tools/push style var.h"

using namespace cheat;
using namespace hooks;
using namespace gui;
using namespace input;
using namespace utl;

wndproc::wndproc( )
{
}

bool wndproc::Do_load( )
{
	auto hwnd = user_input::get_shared( )->hwnd( );

	const bool unicode = IsWindowUnicode(hwnd);
	const auto game_wndproc = reinterpret_cast<WNDPROC>(invoke(unicode ? GetWindowLongPtrW : GetWindowLongPtrA, hwnd, GWLP_WNDPROC));

	default_wndproc__ = unicode ? DefWindowProcW : DefWindowProcA;
	target_func_ = method_info::make_static(game_wndproc);

	this->hook( );
	this->enable( );

	return true;
}

void wndproc::Callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef CHEAT_GUI_TEST
	if (wparam == VK_DELETE && msg == WM_KEYUP)
	{
		this->disable( );
		this->return_value_.store_value(TRUE);
		services_loader::get().unload( );
		return;
	}
#endif

	using result = user_input::process_result;

	switch (user_input::get_shared()->process(hwnd, msg, wparam, lparam))
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
		default:
		{
			if (override_return__)
				this->return_value_.store_value(override_return_to__);
			break;
		}
	}
}

void wndproc::render( )
{
	ImGui::Checkbox("override return", &override_return__);
	if (override_return__)
	{
		const auto pop = memory_backup(ImGui::GetStyle( ).ItemSpacing.x, 0);
		(void)pop;

		ImGui::SameLine( );
		ImGui::Text(" to ");
		ImGui::SameLine( );
		if (ImGui::RadioButton("0 ", override_return_to__ == 0))
			override_return_to__ = 0;
		ImGui::SameLine( );
		if (ImGui::RadioButton("1", override_return_to__ == 1))
			override_return_to__ = 1;
	}
}
