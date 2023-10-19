#include "render/backend/native_win32.h"
#include "functional/ignore.h"

#include <imgui_impl_win32.h>

#include <Windows.h>
#include <tchar.h>

namespace fd
{
class find_window_helper : public noncopyable
{
    HWND console_;
    DWORD current_process_id_;

    HWND target_;

  public:
    find_window_helper(HWND console = GetConsoleWindow(), DWORD const current_process = GetCurrentProcessId())
        : console_(console)
        , current_process_id_(current_process)
    {
        ignore_unused(target_);
    }

    bool compare(HWND current_window)
    {
        if (GetWindow(current_window, GW_OWNER))
            return false;
        if (!IsWindowVisible(current_window))
            return false;
        if (current_window == console_)
            return false;
        DWORD process_id;
        GetWindowThreadProcessId(current_window, &process_id);
        if (current_process_id_ != process_id)
            return false;

        target_ = current_window;

        return true;
    }

    HWND get() const
    {
        return target_;
    }
};

static HWND find_main_window() noexcept
{
    find_window_helper helper;
    auto const found = EnumWindows(
        [](HWND current_window, LPARAM const lparam) {
            auto const helper_ptr = reinterpret_cast<find_window_helper*>(lparam);
            return helper_ptr->compare(current_window) ? FALSE : TRUE;
        },
        reinterpret_cast<LPARAM>(&helper));
    assert(found == TRUE);
    return helper.get();
}

native_win32_backend::native_win32_backend()
    : native_win32_backend(find_main_window())
{
}

native_win32_backend::native_win32_backend(HWND window)
    : basic_win32_backend(window)
    , window_(window)
{
}

win32_window_info native_win32_backend::info() const
{
    return {window_};
}
} // namespace fd