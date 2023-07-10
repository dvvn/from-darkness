#include "win32.h"
#include "noncopyable.h"
#include "diagnostics/system_error.h"

#include <imgui_impl_win32.h>

#include <Windows.h>
#include <tchar.h>

namespace fd
{
template <bool Validate>
static HWND find_game_window() noexcept(!Validate)
{
    auto window = FindWindow(_T("Valve001"), nullptr);
    if constexpr (Validate)
        if (!window)
            throw system_error("Game window not found!");
    return window;
}

class native_win32_backend final : public basic_win32_backend, public noncopyable
{
    HWND window_;

    native_win32_backend(HWND window)
        : basic_win32_backend(window)
        , window_(window)
    {
    }

  public:
    ~native_win32_backend() override
    {
        native_win32_backend::destroy();
    }

    native_win32_backend()
        : native_win32_backend(find_game_window<true>())
    {
    }

    void destroy() override
    {
        if (IsWindow(window_))
            basic_win32_backend::destroy();
    }

#ifdef _DEBUG
    update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam) override
    {
        assert(window_ == window);
        return basic_win32_backend::update(window, message, wparam, lparam);
    }
#endif

    WNDPROC proc() const override
    {
        return reinterpret_cast<WNDPROC>(GetWindowLongPtr(window_, GWL_WNDPROC));
    }

    HWND id() const override
    {
        return window_;
    }

    bool minimized() const override
    {
        return IsIconic(window_);
    }

    window_size size() const override
    {
        RECT rect;
        auto rect_get = GetWindowRect(window_, &rect);
        assert(rect_get);
        return rect;
    }
};

FD_INTERFACE_IMPL(native_win32_backend);
} // namespace fd