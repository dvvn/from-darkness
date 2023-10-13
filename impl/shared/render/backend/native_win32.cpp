#include "native_win32.h"
//
#include "diagnostics/system_error.h"

#include <Windows.h>
#include <imgui_impl_win32.h>
#include <tchar.h>

namespace fd
{
HWND native_win32_backend::find_game_window()
{
    HWND window;
    auto const found = EnumWindows(
        [](HWND current_window, LPARAM const lparam) {
            if (GetWindow(current_window, GW_OWNER))
                return TRUE;
            if (!IsWindowVisible(current_window))
                return TRUE;
#ifdef _DEBUG
            if (current_window == GetConsoleWindow())
                return TRUE;
#endif
            DWORD process_id;
            GetWindowThreadProcessId(current_window, &process_id);
            if (GetCurrentProcessId() != process_id)
                return TRUE;

            *reinterpret_cast<HWND*>(lparam) = current_window;
            return FALSE;
        },
        reinterpret_cast<LPARAM>(&window));
    if (!found)
        throw system_error("Unable to find game window!");
    return window;
}

native_win32_backend::native_win32_backend(HWND window)
    : basic_win32_backend(window)
    , window_(window)
{
}

void native_win32_backend::fill(win32_window_info* backend_info) const
{
    backend_info->id = window_;
}
} // namespace fd