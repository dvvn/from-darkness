module;

#include <imgui_internal.h>

export module fd.gui.context;

namespace fd::gui
{
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

    export class context
    {
        imgui_backup backup_;
        ImGuiContext context_;
        ImFontAtlas font_atlas_;

      public:
        ~context();
        context(const bool store_settings);

        /*context(const context&)            = delete;
        context& operator=(const context&) = delete;*/
    };
} // namespace fd::gui
