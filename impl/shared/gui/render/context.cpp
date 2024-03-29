﻿#include "gui/render/context.h"

#include <memory>

namespace fd::gui
{
render_context::~render_context()
{
    ImGui::Shutdown();
#ifdef _DEBUG
    ImGui::SetCurrentContext(nullptr);
#endif
}

render_context::render_context()
    : context_{&font_atlas_}
{
#ifdef _DEBUG
    IMGUI_CHECKVERSION();
#endif
    ImGui::SetCurrentContext(&context_);

#if defined(_DEBUG) || defined(IMGUI_DISABLE_DEFAULT_ALLOCATORS)
    ImGui::SetAllocatorFunctions(
        [](size_t const size, void*) {
            return operator new(size, std::nothrow);
        },
        [](void* buff, void*) {
            operator delete(buff, std::nothrow);
        });
#endif

    ImGui::Initialize();

    /*assert(ImGui::FindSettingsHandler("Window"));
   ImGui::RemoveSettingsHandler("Window");*/
    context_.SettingsHandlers.clear_destruct();
    context_.IO.IniFilename = nullptr;

    ImGui::StyleColorsDark();
}

#if 0
//this method sucks
bool render_context::skip_scene() const
{
#ifdef IMGUI_HAS_VIEWPORT
    return false;
#else
    // sets in win32 impl
    auto &display_size = context_.IO.DisplaySize;
    return static_cast<size_t>(display_size.x) != 0 && static_cast<size_t>(display_size.y) != 0;
#endif
}
#endif

void render_context::begin_frame()
{
    // backend call

    /*for (auto w : context.WindowsFocusOrder)
  {
      if (!w->Hidden)
          return true;
      if (w->Active)
          return true;
      if (w->Collapsed)
          return true;
  }*/

    ImGui::NewFrame();
}

void render_context::end_frame()
{
    ImGui::EndFrame();
    ImGui::Render();

    // backend call
}

ImDrawData* render_context::data()
{
    return ImGui::GetDrawData();
}
} // namespace fd::gui