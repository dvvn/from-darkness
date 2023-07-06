#include "context.h"
//
#include "diagnostics/runtime_error.h"
#include "functional/ignore.h"

namespace fd
{
render_context::~render_context()
{
    ImGui::Shutdown();
#ifdef _DEBUG
    ImGui::SetCurrentContext(nullptr);
#endif
}

render_context::render_context()
    : context_(&font_atlas_)
{
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(&context_);

#if defined(_DEBUG) || defined(IMGUI_DISABLE_DEFAULT_ALLOCATORS)
    ImGui::SetAllocatorFunctions(
        [](size_t size, void *) { return operator new(size, std::nothrow); },
        [](void *buff, void *) { operator delete(buff, std::nothrow); });
#endif

    ImGui::Initialize();

    /*assert(ImGui::FindSettingsHandler("Window"));
   ImGui::RemoveSettingsHandler("Window");*/
    context_.SettingsHandlers.clear();
    context_.IO.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    /*if (!render_backend::init(backend))
        throw runtime_error("Unable to init render backend!");
    state_ |= init_state::render;
    if (!system_->init(window))
        throw runtime_error("Unable to init Win32 backend!");*/
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

void render_context::begin_scene()
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

void render_context::end_scene()
{
    ignore_unused(this);
    ImGui::Render();
    // backend call
}

ImDrawData *render_context::data()
{
    return ImGui::GetDrawData();
}

} // namespace fd