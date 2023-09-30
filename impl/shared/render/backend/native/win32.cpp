﻿#include "noncopyable.h"
#include "object_holder.h"
#include "win32.h"
#include "diagnostics/system_error.h"

#include <Windows.h>
#include <imgui_impl_win32.h>
#include <tchar.h>

namespace fd
{
template <bool Validate>
static HWND find_game_window() noexcept(!Validate)
{
    auto const window = FindWindow(_T("Valve001"), nullptr);
    if constexpr (Validate)
        if (!window)
            throw system_error("Game window not found!");
    return window;
}

class native_win32_backend final : public basic_win32_backend, public noncopyable
{
    HWND window_;

  public:
    ~native_win32_backend() override
    {
        native_win32_backend::destroy();
    }

    native_win32_backend(HWND window)
        : window_(window)
    {
        basic_win32_backend::setup(window);
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

    void update(win32_backend_info* backend_info) const override
    {
        backend_info->id = window_;
    }
};

basic_win32_backend* make_incomplete_object<native_win32_backend>::operator()() const
{
    return make_object<native_win32_backend>();
}
} // namespace fd