#include "win32.h"
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

class win32_backend_native final : public basic_win32_backend, public noncopyable
{
#ifdef _DEBUG
    HWND window_;

    win32_backend_native(HWND window)
        : basic_win32_backend(window)
        , window_(window)
    {
    }
#endif

  public:
    ~win32_backend_native() override
    {
        destroy();
    }

    win32_backend_native()
        :
#ifdef _DEBUG
        win32_backend_native
#else
        basic_win32_backend
#endif
        (find_game_window<true>())
    {
    }

    void destroy() override
    {
        bool exists;
#ifdef _DEBUG
        exists = IsWindow(window_);
#else
        exists = find_game_window<false>();
#endif

        if (exists)
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
        return reinterpret_cast<WNDPROC>(GetWindowLongPtr(id(), GWL_WNDPROC));
    }

    HWND id() const override
    {
#ifdef _DEBUG
        return window_;
#else
        return find_game_window<true>();
#endif
    }
};

FD_INTERFACE_IMPL(win32_backend_native);
} // namespace fd