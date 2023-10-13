#include "basic_win32.h"
//
#include "diagnostics/runtime_error.h"

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

// ReSharper disable CppInconsistentNaming

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// ReSharper restore CppInconsistentNaming

namespace fd
{
simple_win32_window_size::simple_win32_window_size()
    : w(CW_USEDEFAULT)
    , h(CW_USEDEFAULT)
{
}

simple_win32_window_size::simple_win32_window_size(RECT const& rect)
    : w(rect.right - rect.left)
    , h(rect.bottom - rect.top)
{
}

win32_window_size::win32_window_size()
    : x(CW_USEDEFAULT)
    , y(CW_USEDEFAULT)
{
}

win32_window_size::win32_window_size(RECT const& rect)
    : simple_win32_window_size(rect)
    , x(rect.top)
    , y(rect.left)
{
}

win32_window_size& win32_window_size::operator=(simple_win32_window_size const& parent_size)
{
    simple_win32_window_size::operator=(parent_size);
    return *this;
}

WNDPROC win32_window_info::proc() const
{
    return reinterpret_cast<WNDPROC>(GetWindowLongPtr(id, GWL_WNDPROC));
}

win32_window_size win32_window_info::size() const
{
    RECT rect;
    /*GetWindowRect*/ GetClientRect(id, &rect);
    return rect;
}

bool win32_window_info::minimized() const
{
    return IsIconic(id);
}

static_win32_window_info::static_win32_window_info(HWND id)
    : static_win32_window_info(win32_window_info(id))
{
}

static_win32_window_info::static_win32_window_info(win32_window_info info)
    : dynamic(std::move(info))
    , proc(dynamic.proc())
    , size(dynamic.size())
    , minimized(dynamic.minimized())
{
}

basic_win32_backend::~basic_win32_backend()
{
    ImGui_ImplWin32_Shutdown();
}

basic_win32_backend::basic_win32_backend(HWND window)
{
    if (!ImGui_ImplWin32_Init(window))
        throw runtime_error("Unable to init ImGui_ImplWin32!");
}

void basic_win32_backend::new_frame()
{
    ImGui_ImplWin32_NewFrame();
}

win32_backend_update_finish basic_win32_backend::update(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    assert(message != WM_DESTROY);

    auto const& events       = GImGui->InputEventsQueue;
    auto const events_stored = events.size();

    auto value = ImGui_ImplWin32_WndProcHandler(window, message, wparam, lparam);
    win32_backend_update_response response;

    using enum win32_backend_update_response;
    if (value != 0)
        response = locked;
    else if (events_stored != events.size())
        response = updated;
    else
        response = skipped;

    return {response, window, message, wparam, lparam, value};
}

} // namespace fd