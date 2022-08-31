
#include <fd/object.h>

#include <Windows.h>

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

import fd.gui.basic_input_handler;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

using namespace fd::gui;

struct input_handler_impl final : basic_input_handler
{
    ~input_handler_impl() override
    {
        ImGui_ImplWin32_Shutdown();
    }

    input_info operator()(const HWND window, const UINT message, const WPARAM w_param, const LPARAM l_param) override
    {
        const ImGuiContext& ctx = *FD_OBJECT_GET(ImGuiContext*);

        static const auto once = ImGui_ImplWin32_Init(window);

        using result_t = input_info::result_type;

        const auto size_before  = ctx.InputEventsQueue.size();

        if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
            return { result_t::interacted, TRUE };

        const auto size_after = ctx.InputEventsQueue.size();

        if (size_before != size_after)
            return { result_t::interacted /* , FALSE */ };

        return result_t::skipped;
    }
};

FD_OBJECT_BIND_TYPE(input_handler, input_handler_impl);
