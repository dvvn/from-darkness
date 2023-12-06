#pragma once

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <Windows.h>

// ReSharper disable CppInconsistentNaming

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// ReSharper restore CppInconsistentNaming

namespace fd::gui
{
enum class win32_backend_update_response : uint8_t
{
    /**
     * \brief system backend does nothing
     */
    skipped,
    /**
     * \brief system backend does something
     */
    updated,
    /**
     * \copydoc updated, message processing blocked
     */
    locked,
};

struct win32_backend_update_result
{
    win32_backend_update_response response;
    LRESULT retval;
};

class basic_win32_backend
{
  protected:
    ~basic_win32_backend()
    {
        ImGui_ImplWin32_Shutdown();
    }

    basic_win32_backend(HWND window)
    {
        assert(IsWindow(window));
        if (!ImGui_ImplWin32_Init(window))
            assert(0 && "Unable to init ImGui_ImplWin32!");
    }

  public:
    static void new_frame()
    {
        ImGui_ImplWin32_NewFrame();
    }

    static win32_backend_update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
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
};
} // namespace fd::gui