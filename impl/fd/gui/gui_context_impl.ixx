module;

#include <imgui_internal.h>

#include <d3d9.h>

export module fd.gui.context.impl;
export import fd.gui.context;

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

    export class context_impl : public basic_context
    {
      public:
        using backend_type   = IDirect3DDevice9*;
        using window_id_type = HWND;

      private:
        imgui_backup backup_;
        ImGuiContext context_;
        ImFontAtlas font_atlas_;
#ifdef _DEBUG
        backend_type backend_;
        window_id_type window_id_;
#endif

      public:
        ~context_impl() override;
        context_impl(const backend_type backend, const window_id_type window_id, const bool store_settings);

        /*context(const context&)            = delete;
        context& operator=(const context&) = delete;*/

        void release_textures() override;
        bool begin_frame() override;
        void end_frame() override;
        char process_keys(void* data) override;
    };
} // namespace fd::gui
