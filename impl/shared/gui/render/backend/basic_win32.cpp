#include "gui/render/backend/basic_win32.h"

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

// ReSharper disable CppInconsistentNaming

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// ReSharper restore CppInconsistentNaming

namespace fd::gui
{
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

win32_backend_update_result basic_win32_backend::update(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    assert(message != WM_DESTROY);

    auto const& events       = ImGui::GetCurrentContext()->InputEventsQueue;
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
