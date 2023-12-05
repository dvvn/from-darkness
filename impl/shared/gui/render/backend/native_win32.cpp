#include "gui/render/backend/native_win32.h"

#include <Windows.h>

#include <utility>

namespace fd::gui
{
HWND basic_native_win32_backend::find_main_window() noexcept
{
    std::pair compare_data{
        HWND{}, //
        [console = GetConsoleWindow(), current_process = GetCurrentProcessId()](HWND current_window) -> bool {
            if (current_window == console)
                return false;
            if (GetWindow(current_window, GW_OWNER))
                return false;
            if (!IsWindowVisible(current_window))
                return false;
            DWORD process_id;
            GetWindowThreadProcessId(current_window, &process_id);
            if (current_process != process_id)
                return false;

            return true;
        }};

    EnumWindows(
        [](HWND current_window, LPARAM const lparam) {
            auto& [current_window_stored, compare_fn] = *reinterpret_cast<decltype(compare_data)*>(lparam);

            BOOL const check_next = compare_fn(current_window) == false;
            if (!check_next)
                current_window_stored = current_window;

            return check_next;
        },
        reinterpret_cast<LPARAM>(&compare_data));

    return get<HWND>(compare_data);
}

basic_native_win32_backend::basic_native_win32_backend()
    : basic_native_win32_backend(find_main_window())
{
}

basic_native_win32_backend::basic_native_win32_backend(HWND window)
    : basic_win32_backend(window)
    , window_(window)
{
}

HWND basic_native_win32_backend::window() const
{
    return window_;
}
} // namespace fd::gui
