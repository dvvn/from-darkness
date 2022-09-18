module;

#include <imgui_internal.h>

export module fd.gui.context;

namespace fd::gui
{
#if 0
    class imgui_backup
    {
        ImGuiMemAllocFunc allocator_;
        ImGuiMemFreeFunc deleter_;
        void* user_data_;
        ImGuiContext* context_;

      public:
        ~imgui_backup();
        imgui_backup();

        imgui_backup(const imgui_backup&)            = delete;
        imgui_backup& operator=(const imgui_backup&) = delete;
    };
#endif

    export class context
    {
#if 0
        imgui_backup backup_;
#endif
        ImGuiContext context_;
        ImFontAtlas font_atlas_;

      public:
        ~context();
        context();
    };
} // namespace fd::gui
