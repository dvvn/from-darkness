#include "context_init.h"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <new>

namespace fd
{
bool init(render_context *ctx, render_backend backend, HWND window)
{
    IMGUI_CHECKVERSION();

#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions(
        [](size_t size, void * /*user_data*/) { return operator new(size, std::nothrow); },
        [](void *buff, void * /*user_data*/) { operator delete(buff, std::nothrow); });
#endif

    ImGui::SetCurrentContext(&ctx->context);
    ImGui::Initialize();

    assert(ImGui::FindSettingsHandler("Window"));
    ImGui::RemoveSettingsHandler("Window");

    ImGui::StyleColorsDark();

    if (!ImGui_ImplDX9_Init(backend))
        return false;
    ctx->backend = backend;

    if (!ImGui_ImplWin32_Init(window))
        return false;
    ctx->window = window;

    return true;
}

}