#include "gui/render/backend/basic_win32.h"

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

// ReSharper disable CppInconsistentNaming

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// ReSharper restore CppInconsistentNaming

namespace fd::gui
{
win32_window_size_simple::win32_window_size_simple()
    : w(CW_USEDEFAULT)
    , h(CW_USEDEFAULT)
{
}

win32_window_size_simple::win32_window_size_simple(RECT const& rect)
    : w(rect.right - rect.left)
    , h(rect.bottom - rect.top)
{
}

win32_window_size_simple::win32_window_size_simple(LONG w, LONG h)
    : w(w)
    , h(h)
{
}

bool win32_window_size_simple::operator==(win32_window_size_simple const& other) const
{
#if LONG_MAX == INT_MAX
    if constexpr (sizeof(win32_window_size_simple) == sizeof(uint64_t))
        return *reinterpret_cast<uint64_t const*>(this) == reinterpret_cast<uint64_t const&>(other);
    else
#endif
        return w == other.w && h == other.h;
}

win32_window_size::win32_window_size()
    : x(CW_USEDEFAULT)
    , y(CW_USEDEFAULT)
{
}

win32_window_size::win32_window_size(RECT const& rect)
    : win32_window_size_simple(rect)
    , x(rect.top)
    , y(rect.left)
{
}

win32_window_size& win32_window_size::operator=(win32_window_size_simple const& parent_size)
{
    win32_window_size_simple::operator=(parent_size);
    return *this;
}

WNDPROC win32_window_info::proc() const
{
    return reinterpret_cast<WNDPROC>(GetWindowLongPtr(id, GWLP_WNDPROC));
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

win32_window_info_static::win32_window_info_static(HWND id)
    : win32_window_info_static(win32_window_info(id))
{
}

win32_window_info_static::win32_window_info_static(win32_window_info info)
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
    assert(IsWindow(window));
    if (!ImGui_ImplWin32_Init(window))
        assert(0 && "Unable to init ImGui_ImplWin32!");
}

void basic_win32_backend::new_frame()
{
    ImGui_ImplWin32_NewFrame();
}

win32_backend_update_result basic_win32_backend::update(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    assert(message != WM_DESTROY);

    auto const& events       = GImGui->InputEventsQueue;
    auto const events_stored = events.size();

    auto const retval = ImGui_ImplWin32_WndProcHandler(window, message, wparam, lparam);
    win32_backend_update_response response;

    using enum win32_backend_update_response;
    if (retval != 0)
        response = locked;
    else if (events_stored != events.size())
        response = updated;
    else
        response = skipped;

    return {response, retval};
}

} // namespace fd::gui