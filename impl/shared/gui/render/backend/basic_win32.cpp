#include "gui/render/backend/basic_win32.h"

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

// ReSharper disable CppInconsistentNaming

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// ReSharper restore CppInconsistentNaming

namespace fd::gui
{
namespace detail
{
win32_backend_update_result::win32_backend_update_result(size_type const events_before, LRESULT const retval, size_type const events_after)
    : events_before_{events_before}
    , retval_{retval}
    , events_after_{events_after}
{
#ifdef _DEBUG
    static_assert(std::is_same_v<size_type, decltype(ImGui::GetCurrentContext()->InputEventsQueue.size())>);
#endif
}

LRESULT win32_backend_update_result::retval_or_default(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam) const
{
    LRESULT current_retval;
    if (locked())
        current_retval = retval_;
    else
        current_retval = DefWindowProc(window, message, wparam, lparam);
    return current_retval;
}

LRESULT win32_backend_update_result::retval_or_default_or_original(
    HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam, WNDPROC original) const
{
    return retval_or_default_or_original<WNDPROC>(window, message, wparam, lparam, original);
}

bool win32_backend_update_result::locked() const
{
    return retval_ != 0;
}

bool win32_backend_update_result::updated() const
{
    return events_before_ != events_after_;
}

LRESULT win32_backend_update_result::get() const
{
    return retval_;
}
} // namespace detail

basic_win32_backend::~basic_win32_backend()
{
    ImGui_ImplWin32_Shutdown();
}

basic_win32_backend::basic_win32_backend(HWND window)
{
    assert(IsWindow(window));
    auto const init_result = ImGui_ImplWin32_Init(window);
    assert(init_result == true); // Unable to init ImGui_ImplWin32!
}

void basic_win32_backend::new_frame()
{
    ImGui_ImplWin32_NewFrame();
}

auto basic_win32_backend::update(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam) -> update_result
{
    auto const& events = ImGui::GetCurrentContext()->InputEventsQueue;

    auto const events_count_before = events.size();
    auto const retval              = update_simple(window, message, wparam, lparam);
    auto const events_count_after  = events.size();

    return {events_count_before, retval, events_count_after};
}

LRESULT basic_win32_backend::update_simple(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    assert(message != WM_DESTROY);
    return ImGui_ImplWin32_WndProcHandler(window, message, wparam, lparam);
}
} // namespace fd::gui
