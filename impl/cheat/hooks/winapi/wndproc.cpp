module;

#include <windows.h>

#include <string_view>

module cheat.hooks.winapi.wndproc;
import cheat.hooks.hook;
import cheat.gui.context;
import cheat.tools.object_name;

//#define HOT_UNLOAD_SUPPORTED

#ifdef HOT_UNLOAD_SUPPORTED
import cheat.hooks;
#endif

using namespace cheat;
using namespace hooks;
using namespace winapi;

struct wndproc_impl final : wndproc, hook, hook_instance_static<wndproc_impl>
{
	using wndproc_t = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);

	wndproc_impl( )
	{
		wndproc_t curr;
		const HWND hwnd = nstd::instance_of<gui::context>->get_info( ).window;
		const auto unicode = IsWindowUnicode(hwnd);

		def_ = unicode?DefWindowProcW:DefWindowProcA;
		curr = (wndproc_t)std::invoke(unicode?GetWindowLongPtrW:GetWindowLongPtrA, hwnd, GWLP_WNDPROC);

		//---

		entry_type entry;
		entry.set_target_method(curr);
		entry.set_replace_method(&callback);

		this->init(std::move(entry));
	}

	static LRESULT WINAPI callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
	{
#define ARGS hwnd, msg, wparam, lparam
		const auto block_input = nstd::instance_of<gui::context>->input(ARGS).touched( );
		return block_input ? get( ).def_(ARGS) : call_original(ARGS);
	}

private:
	wndproc_t def_;
};

std::string_view wndproc::function_name( ) const noexcept
{
	return tools::object_name<wndproc>;
}

template<>
template<>
nstd::one_instance_getter<wndproc*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(wndproc_impl::get_ptr( ))
{
}
