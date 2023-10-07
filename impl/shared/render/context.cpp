#include "context.h"
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
    context_.SettingsHandlers.clear();
    context_.IO.IniFilename = nullptr;

    ImGui::StyleColorsDark();
}

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

ImDrawData* render_context::data() const
{
    return ImGui::GetDrawData();
}
} // namespace fd