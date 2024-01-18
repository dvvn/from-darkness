#include "functional/cast.h"
#include "gui/render/backend/native_win32.h"

namespace fd::gui
{
namespace detail
{
template <bool HaveConsole>
class find_native_main_window_helper
{
    using console_type = std::conditional_t<HaveConsole, HWND, std::false_type>;

    [[no_unique_address]] console_type console_;
    DWORD current_process_;

    static console_type default_console_value() noexcept
    {
        if constexpr (HaveConsole)
            return GetConsoleWindow();
        else
            return {};
    }

  public:
    find_native_main_window_helper(console_type console = default_console_value()) noexcept
        : console_{console}
        , current_process_{GetCurrentProcessId()}
    {
    }

    bool operator()(HWND current_window) const noexcept
    {
        if constexpr (HaveConsole)
            if (console_ == current_window)
                return false;
        if (GetWindow(current_window, GW_OWNER))
            return false;
        if (!IsWindowVisible(current_window))
            return false;
        DWORD process_id;
        GetWindowThreadProcessId(current_window, &process_id);
        if (current_process_ != process_id)
            return false;
        return true;
    }
};

template <bool HaveConsole>
struct find_native_main_window_data
{
    find_native_main_window_helper<HaveConsole> compare;
    HWND window = nullptr;
};

find_native_main_window_data(HWND) -> find_native_main_window_data<true>;
find_native_main_window_data() -> find_native_main_window_data<false>;

static HWND find_native_main_window() noexcept
{
    auto const enum_windows_proxy = []<class T>(T data) {
        EnumWindows(
            [](HWND current_window, LPARAM const lparam) -> BOOL {
                T* const data_ptr = unsafe_cast_from(lparam);
                if (!data_ptr->compare(current_window))
                    return FALSE;
                data_ptr->window = current_window;
                return TRUE;
            },
            unsafe_cast_from(&data));

        return data.window;
    };

    HWND console;
#ifdef _DEBUG
    console = GetConsoleWindow();
#else
    console = nullptr;
#endif
    return console ? enum_windows_proxy(find_native_main_window_data{console}) : enum_windows_proxy(find_native_main_window_data{});
}
} // namespace detail

basic_native_win32_backend::basic_native_win32_backend(HWND window)
    : basic_win32_backend{window}
    , window_{window}
{
}

basic_native_win32_backend::basic_native_win32_backend()
    : basic_native_win32_backend{detail::find_native_main_window()}
{
}

win::window_info basic_native_win32_backend::window() const
{
    return window_;
}
} // namespace fd::gui
