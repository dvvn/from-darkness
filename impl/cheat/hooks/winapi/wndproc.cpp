#include "wndproc.h"
#ifndef CHEAT_GUI_TEST
#include "cheat/core/services_loader.h"
#endif
#include "cheat/gui/menu.h"

#include <Windows.h>

using namespace cheat;
using namespace hooks::winapi;
using namespace gui;

// ReSharper disable once CppInconsistentNaming
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void wndproc_impl::callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef CHEAT_GUI_TEST
	if (wparam == VK_DELETE && msg == WM_KEYUP)
	{
		this->disable( );
		this->return_value_.store_value(TRUE);
		services_loader::get_ptr( )->unload_delayed( );
		return;
	}
#endif

	enum class result : uint8_t
	{
		none
	  , blocked
	  , skipped
	};

	// ReSharper disable once CppTooWideScopeInitStatement
	const auto owerride_input = [&]
	{
		const auto& menu = menu::get( );

		const auto skip_input = [&]
		{
			//todo: if skipped -> render last filled buffer
			switch (msg)
			{
				case WM_CLOSE:
				case WM_DESTROY:
				case WM_QUIT:
				case WM_SYSCOMMAND:
				case WM_MOVE:
				case WM_SIZE:
				case WM_KILLFOCUS:
				case WM_SETFOCUS:
				case WM_FONTCHANGE:
				case WM_ACTIVATE:
					return result::none;
				default:
					return result::skipped;
			}
		};

		if (menu->toggle(msg, wparam) || menu->updating( ))
		{
			return skip_input( );
		}

#ifndef  CHEAT_GUI_TEST
		if (menu->visible( ))
#endif
		{
			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
				return result::blocked;
			return skip_input( );
		}
		// ReSharper disable once CppUnreachableCode
		return result::none;
	};

	switch (owerride_input( ))
	{
		case result::blocked:
		{
			this->return_value_.store_value(TRUE);
			break;
		}
		case result::skipped:
		{
			this->return_value_.store_value(default_wndproc_(hwnd, msg, wparam, lparam));
			break;
		}
		default:
		{
			if (override_return_)
				this->return_value_.store_value(override_return_to_);
			break;
		}
	}
}
#if 0
void wndproc_impl::render( )
{
	ImGui::Checkbox("override return", &override_return_);
	if (override_return_)
	{
		const auto pop = nstd::mem::backup(ImGui::GetStyle( ).ItemSpacing.x, 0.f);
		(void)pop;

		ImGui::SameLine( );
		ImGui::Text(" to ");
		ImGui::SameLine( );
		if (ImGui::RadioButton("0 ", override_return_to_ == 0))
			override_return_to_ = 0;
		ImGui::SameLine( );
		if (ImGui::RadioButton("1", override_return_to_ == 1))
			override_return_to_ = 1;
	}
}
#endif
