#include "context_update.h"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

// ReSharper disable once CppInconsistentNaming
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

namespace fd
{
void reset(render_context_ptr ctx)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void resize(render_context_ptr ctx, UINT w, UINT h)
{
}

void process_message(render_context_ptr ctx, UINT message, WPARAM wParam, LPARAM lParam, process_message_result *result)
{
    // reserved, WIP
    auto process = true;

    if (!process)
    {
        if (result)
            *result = process_message_result::idle;
        return;
    }

    if (!result)
    {
        ImGui_ImplWin32_WndProcHandler(ctx->window, message, wParam, lParam);
        return;
    }

    auto &events       = ctx->context.InputEventsQueue;
    auto events_stored = events.size();

    if (ImGui_ImplWin32_WndProcHandler(ctx->window, message, wParam, lParam) != 0)
        *result = process_message_result::locked;
    else if (events_stored != events.size())
        *result = process_message_result::updated;
    else
        *result = process_message_result::idle;
}
}