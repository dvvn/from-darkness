module;
#include <nstd/type name.h>
#include <nstd/one_instance.h>
#include <nstd/runtime_assert.h>
#include <dhooks/wrapper.h>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/async_mutex.hpp>
#include <imgui_impl_win32.h>
#include <Windows.h>

module cheat.hooks.winapi.wndproc;
import cheat.gui.menu;
import cheat.core.services_loader;

using namespace cheat;
using namespace hooks::winapi;

wndproc::wndproc( )
{
	this->add_dependency(gui::context::get( ));
}

bool wndproc::load_impl( ) noexcept
{
	const auto hwnd = gui::context::get( )->hwnd( );
	runtime_assert(hwnd != nullptr);

	unicode_ = IsWindowUnicode(hwnd);
	default_wndproc_ = unicode_ ? DefWindowProcW : DefWindowProcA;

	return basic_service::load_impl( );
}

void* wndproc::get_target_method( ) const
{
	const auto val = std::invoke(unicode_ ? GetWindowLongPtrW : GetWindowLongPtrA, gui::context::get( )->hwnd( ), GWLP_WNDPROC);
	return reinterpret_cast<void*>(val);
}

// ReSharper disable once CppInconsistentNaming
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void wndproc::callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef CHEAT_GUI_TEST
	if (wparam == VK_DELETE && msg == WM_KEYUP)
	{
		this->disable( );
		this->store_return_value(TRUE);
		services_loader::get( ).unload_delayed( );
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
		if (gui::context::get( ).inctive( ))
			return result::none;

		const auto menu = gui::menu::get( ).operator->( );

		if (menu->toggle(msg, wparam))
			return result::skipped;

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

		if (menu->updating( ))
			return can_skip_input(true) ? result::skipped : result::none;

#ifndef  CHEAT_GUI_TEST
		if (menu->visible( ))
#endif
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
	default:
	{
		if (override_return_)
			this->store_return_value(override_return_to_);
		break;
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
