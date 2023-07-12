#include "context.h"
#include "noncopyable.h"
#include "functional/ignore.h"

#include <imgui_internal.h>

namespace fd
{
class render_context final : public basic_render_context, public noncopyable
{
    ImFontAtlas font_atlas_;
    ImGuiContext context_;

  public:
    ~render_context() override
    {
        ImGui::Shutdown();
#ifdef _DEBUG
        ImGui::SetCurrentContext(nullptr);
#endif
    }

    render_context()
        : context_(&font_atlas_)
    {
        IMGUI_CHECKVERSION();
        ImGui::SetCurrentContext(&context_);

#if defined(_DEBUG) || defined(IMGUI_DISABLE_DEFAULT_ALLOCATORS)
        ImGui::SetAllocatorFunctions(
            [](size_t size, void *) { return operator new(size, std::nothrow); },
            [](void *buff, void *) { operator delete(buff, std::nothrow); }
        );
#endif

        ImGui::Initialize();

        /*assert(ImGui::FindSettingsHandler("Window"));
       ImGui::RemoveSettingsHandler("Window");*/
        context_.SettingsHandlers.clear();
        context_.IO.IniFilename = nullptr;

        ImGui::StyleColorsDark();
    }

#if 0
//this method sucks
bool skip_scene() const
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

    void begin_frame() override
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

    void end_frame() override
    {
        ImGui::EndFrame();
        ImGui::Render();
        // backend call
    }

    ImDrawData *data() override
    {
        return ImGui::GetDrawData();
    }
};

FD_INTERFACE_IMPL(render_context);
} // namespace fd