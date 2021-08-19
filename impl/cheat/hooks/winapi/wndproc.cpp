#include "wndproc.h"

#include "cheat/core/services loader.h"
#include "cheat/gui/imgui context.h"
#include "cheat/gui/menu.h"
#include "cheat/gui/tools/push style var.h"

using namespace cheat;
using namespace gui;
using namespace hooks;
using namespace winapi;
using namespace utl;

wndproc::wndproc( )
{
}

bool wndproc::Do_load( )
{
	const auto hwnd = imgui_context::get_ptr( )->hwnd( );
	runtime_assert(hwnd != nullptr);
	unicode_ = IsWindowUnicode(hwnd);

	using namespace address_pipe;

	default_wndproc__ = unicode_ ? DefWindowProcW : DefWindowProcA;

	this->hook( );
	this->enable( );

	return true;
}

utl::address wndproc::get_target_method_impl( ) const
{
	return std::invoke(unicode_ ? GetWindowLongPtrW : GetWindowLongPtrA, imgui_context::get_ptr( )->hwnd( ), GWLP_WNDPROC);
}

// ReSharper disable once CppInconsistentNaming
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void wndproc::callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef CHEAT_GUI_TEST
	if (wparam == VK_DELETE && msg == WM_KEYUP)
	{
		this->disable( );
		this->return_value_.store_value(TRUE);
		services_loader::get_ptr( )->unload( );
		return;
	}
#endif

	enum class result : uint8_t
	{
		none,
		blocked,
		skipped
	};

	// ReSharper disable once CppTooWideScopeInitStatement
	const auto owerride_input = [&]
	{
		const auto menu = menu::get_ptr( );

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
				case WM_ACTIVATE:
					return result::none;
				default:
					return result::skipped;
			}
		};

		if (menu->toggle(msg, wparam) || menu->animating( ))
		{
			return skip_input( );
		}

#if !defined(CHEAT_GUI_HAS_DEMO_WINDOW) || !defined(CHEAT_GUI_TEST)
		if (menu->active( ))
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
