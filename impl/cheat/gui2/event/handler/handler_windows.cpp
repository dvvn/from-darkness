module;

#include <nstd/runtime_assert.h>

#include <windows.h>

module cheat.gui2.event.handler.windows;

using namespace cheat;

auto gui2::get_event(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> unique_pointer<event>
{
	runtime_assert("Finish the function!");
}