#include "wndproc.h"

#include "cheat/core/services_loader.h"
#include "cheat/core/console.h"
#include "cheat/gui/imgui_context.h"

#include <windows.h>

using namespace cheat;
using namespace hooks::winapi;

using gui::imgui_context;

wndproc::wndproc( )
{
	this->wait_for_service<imgui_context>( );
}

service_impl::load_result wndproc::load_impl( ) noexcept
{
	const auto hwnd = imgui_context::get_ptr( )->hwnd( );
	runtime_assert(hwnd != nullptr);

	unicode_         = IsWindowUnicode(hwnd);
	default_wndproc_ = unicode_ ? DefWindowProcW : DefWindowProcA;

	CHEAT_HOOK_PROXY_INIT(TRUE)
}

nstd::address wndproc::get_target_method_impl( ) const
{
	return std::invoke(unicode_ ? GetWindowLongPtrW : GetWindowLongPtrA, imgui_context::get_ptr( )->hwnd( ), GWLP_WNDPROC);
}

CHEAT_REGISTER_SERVICE(wndproc);