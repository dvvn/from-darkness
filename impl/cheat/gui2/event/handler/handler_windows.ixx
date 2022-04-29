module;

#include <windows.h>

export module cheat.gui2.event.handler.windows;
export import cheat.gui2.event;

using wndproc_t = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);

export namespace cheat::gui2
{
	unique_pointer<event> get_event(HWND hwnd, UINT message, WPARAM wparam,LPARAM lparam);
}