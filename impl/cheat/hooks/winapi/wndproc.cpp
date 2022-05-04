module;

#include <cheat/hooks/instance.h>

#include <windows.h>

module cheat.hooks.winapi.wndproc;
import cheat.hooks.base;
import cheat.gui.context;
import nstd.one_instance;

#define HOT_UNLOAD_SUPPORTED

#ifdef HOT_UNLOAD_SUPPORTED
import cheat.hooks;
#endif

using namespace cheat;
using namespace hooks;

CHEAT_HOOK_INSTANCE(winapi, wndproc);

using wndproc_t = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);
struct wndproc_info_t
{
	wndproc_t def, curr;

	wndproc_info_t( )
	{
		update( );
	}

	void update(const HWND hwnd = nstd::instance_of<gui::context>->get_info( ).window)
	{
		const auto unicode = IsWindowUnicode(hwnd);
		if(unicode)
		{
			def = DefWindowProcW;
			curr = (wndproc_t)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
		}
		else
		{
			def = DefWindowProcA;
			curr = (wndproc_t)GetWindowLongPtrA(hwnd, GWLP_WNDPROC);
		}
	}
};
constexpr nstd::instance_of_t<wndproc_info_t> wndproc_info;

static void* target( ) noexcept
{
	return wndproc_info->curr;
}

// ReSharper disable once CppInconsistentNaming
//extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

struct replace
{
#if 0
	enum class override_info : uint8_t
	{
		none
		, blocked
		, skipped
		, special
	};

	static override_info override_input(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
	{
		using namespace gui;
		auto& ctx = context::get( );

		const auto window_active = !ctx.inactive( );
		if(!window_active)
			return override_info::none;

		const auto input_active = ctx.IO.WantTextInput;
		if(!input_active)
		{
#ifdef HOT_UNLOAD_SUPPORTED
			const auto unload_wanted = wparam == VK_DELETE && msg == WM_KEYUP;
			if(unload_wanted)
			{
				//disable( );
				//this->store_return_value(TRUE);
				hooks::unload( );
				return override_info::special;
			}
#endif
			if(menu::toggle(msg, wparam))
				return override_info::skipped;
		}

		const auto can_skip_input = [&](bool manual_imgui_handler)
		{
			//todo: if skipped -> render last filled buffer
			switch(msg)
			{
				case WM_KILLFOCUS:
				case WM_SETFOCUS:
					if(manual_imgui_handler)
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

		if(menu::updating( ))
			return can_skip_input(true) ? override_info::skipped : override_info::none;

		if(menu::visible( ))
		{
			if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
				return override_info::blocked;

			if(can_skip_input(false))
				return override_info::skipped;
		}

		return override_info::none;
	}
#endif

	static LRESULT WINAPI fn(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
	{
#if 0
		switch(override_input(hwnd, msg, wparam, lparam))
		{
			case override_info::none:
				return CHEAT_HOOK_CALL_ORIGINAL_STATIC(hwnd, msg, wparam, lparam);
			case override_info::blocked:
				return TRUE;
			case override_info::skipped:
				return CHEAT_HOOK_CALL(wndproc_info->def, hwnd, msg, wparam, lparam);
#ifdef HOT_UNLOAD_SUPPORTED
			case override_info::special:
				return TRUE;
#endif
}
#endif
		if(nstd::instance_of<gui::context>->input(hwnd, msg, wparam, lparam).touched( ))
			return CHEAT_HOOK_CALL(wndproc_info->def, hwnd, msg, wparam, lparam);

		return CHEAT_HOOK_CALL_ORIGINAL_STATIC(hwnd, msg, wparam, lparam);
	}
};

CHEAT_HOOK_INIT(winapi, wndproc);
