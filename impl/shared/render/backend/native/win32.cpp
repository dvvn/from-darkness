#include "win32.h"
//
#include "diagnostics/system_error.h"

#include <imgui_impl_win32.h>

#include <Windows.h>
#include <tchar.h>

namespace fd
{
template <bool Validate>
static HWND find_game_window()
{
    auto window = FindWindow(_T("Valve001"), nullptr);
    if constexpr (Validate)
        if (!window)
            throw system_error("Game window not found!");
    return window;
}

win32_backend_native::~win32_backend_native()
{
    win32_backend_native::destroy();
}

#ifdef _DEBUG
win32_backend_native::win32_backend_native(HWND window)
    : basic_win32_backend(window)
    , window_(window)
{
}
#endif

win32_backend_native::win32_backend_native()
    :
#ifdef _DEBUG
    win32_backend_native
#else
    basic_win32_backend
#endif
    (find_game_window<true>())
{
}

void win32_backend_native::destroy()
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
auto win32_backend_native::update(HWND window, UINT message, WPARAM wparam, LPARAM lparam) -> update_result
{
    assert(window_ == window);
    return basic_win32_backend::update(window, message, wparam, lparam);
}
#endif

} // namespace fd