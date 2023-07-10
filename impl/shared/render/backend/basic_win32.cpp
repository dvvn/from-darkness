#include "basic_win32.h"
#include "diagnostics/runtime_error.h"

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <Windows.h>

// ReSharper disable CppInconsistentNaming

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// ReSharper restore CppInconsistentNaming

namespace fd
{
basic_win32_backend::window_size::window_size()
    : w(0)
    , h(0)
{
}

basic_win32_backend::window_size::window_size(LPARAM lparam)
    : w(LOWORD(lparam))
    , h(HIWORD(lparam))
{
}

basic_win32_backend::window_size::window_size(RECT const &rect)
    : w(rect.right - rect.left)
    , h(rect.bottom - rect.top)
{
}

basic_win32_backend::basic_win32_backend(HWND window)
{
    if (!ImGui_ImplWin32_Init(window))
        throw runtime_error("Unable to init ImGui_ImplWin32!");
}

void basic_win32_backend::destroy()
{
    ImGui_ImplWin32_Shutdown();
}

void basic_win32_backend::new_frame()
{
    ImGui_ImplWin32_NewFrame();
}

auto basic_win32_backend::update(HWND window, UINT message, WPARAM wparam, LPARAM lparam) -> update_result
{
    assert(message != WM_DESTROY);

    auto ctx = ImGui::GetCurrentContext();

    auto &events       = ctx->InputEventsQueue;
    auto events_stored = events.size();

    auto value = ImGui_ImplWin32_WndProcHandler(window, message, wparam, lparam);
    response_type response;

    if (value != 0)
        response = locked;
    else if (events_stored != events.size())
        response = updated;
    else
        response = skipped;

    return {value, response};
}

}