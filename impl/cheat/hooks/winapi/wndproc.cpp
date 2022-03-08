module;

#include "cheat/hooks/base_includes.h"
#include <windows.h>

module cheat.hooks.winapi:wndproc;
import cheat.hooks.loader;
import cheat.gui;

using namespace cheat;
using namespace hooks::winapi;

wndproc::wndproc( )
{
	const auto hwnd = gui::context::get( ).hwnd;
	unicode_ = IsWindowUnicode(hwnd);
	default_wndproc_ = unicode_ ? DefWindowProcW : DefWindowProcA;

	const auto val = std::invoke(unicode_ ? GetWindowLongPtrW : GetWindowLongPtrA, hwnd, GWLP_WNDPROC);
	this->set_target_method(reinterpret_cast<void*>(val));
}

void wndproc::construct( ) noexcept
{
}

// ReSharper disable once CppInconsistentNaming
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void wndproc::callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	using namespace gui;
	auto& ctx = context::get( );

	enum class result : uint8_t
	{
		none
		, blocked
		, skipped
		, special
	};

	// ReSharper disable once CppTooWideScopeInitStatement
	const auto owerride_input = [&]
	{
		const auto window_active = !ctx.inactive( );
		if (!window_active)
			return result::none;

		const auto input_active = ctx.IO.WantTextInput;
		if (!input_active)
		{
#if 0
			const auto unload_wanted = wparam == VK_DELETE && msg == WM_KEYUP;
			if (unload_wanted)
			{
				this->disable( );
				this->store_return_value(TRUE);
				cheat::unload( );
				return result::special;
			}
#endif
			if (menu::toggle(msg, wparam))
				return result::skipped;
		}

		const auto can_skip_input = [&](bool manual_imgui_handler)
		{
			//todo: if skipped -> render last filled buffer
			switch (msg)
			{
			case WM_KILLFOCUS:
			case WM_SETFOCUS:
				if (manual_imgui_handler)
					ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
			case WM_CLOSE:
			case WM_DESTROY:
			case WM_QUIT:
			case WM_SYSCOMMAND:
			case WM_MOVE:
			case WM_SIZE:
			case WM_FONTCHANGE:
			case WM_ACTIVATE:
			case WM_ACTIVATEAPP:
			case WM_ENABLE:
				return false;
			default:
				return true;
			}
		};

		if (menu::updating( ))
			return can_skip_input(true) ? result::skipped : result::none;

		if (menu::visible( ))
		{
			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
				return result::blocked;

			if (can_skip_input(false))
				return result::skipped;
		}

		return result::none;
	};

	switch (owerride_input( ))
	{
	case result::none:
	{
		if (override_return_)
			this->store_return_value(override_return_to_);
		break;
	}
	case result::blocked:
	{
		this->store_return_value(TRUE);
		break;
	}
	case result::skipped:
	{
		this->store_return_value(default_wndproc_(hwnd, msg, wparam, lparam));
		break;
	}
	case result::special:
	{
	}
	}
}

#if 0
void wndproc::render( )
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