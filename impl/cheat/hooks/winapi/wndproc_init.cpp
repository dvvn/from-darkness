#include "wndproc.h"

#include "cheat/core/services_loader.h"
#include "cheat/core/console.h"
#include "cheat/gui/imgui_context.h"

#include <cppcoro/task.hpp>

#include <windows.h>

using namespace cheat;
using namespace hooks::winapi;

using gui::imgui_context;

wndproc_impl::wndproc_impl( )
{
	CHEAT_SERVICE_ADD_SHARED_DEPENDENCY(imgui_context);
}

auto wndproc_impl::load_impl( ) noexcept -> load_result
{
	const auto hwnd = imgui_context::get( )->hwnd( );
	runtime_assert(hwnd != nullptr);

	unicode_         = IsWindowUnicode(hwnd);
	default_wndproc_ = unicode_ ? DefWindowProcW : DefWindowProcA;

	CHEAT_LOAD_HOOK_PROXY;
}

nstd::address wndproc_impl::get_target_method_impl( ) const
{
	const auto val = std::invoke(unicode_ ? GetWindowLongPtrW : GetWindowLongPtrA, imgui_context::get( )->hwnd( ), GWLP_WNDPROC);
	return nstd::address(val);
}

CHEAT_SERVICE_REGISTER(wndproc);
