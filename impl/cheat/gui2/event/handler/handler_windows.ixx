module;

#include <windows.h>

export module cheat.gui2.event.handler.windows;

using wndproc_t = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);

export namespace cheat::gui2
{
	void set_event_handler(wndproc_t handler);
}