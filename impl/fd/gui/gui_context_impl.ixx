module;

#include <imgui_internal.h>

#include <d3d9.h>

#include <vector>

export module fd.gui.context.impl;
export import fd.gui.context;
import fd.functional.fn;

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

    struct keys_pack : std::vector<ImGuiKey>
    {
        void sort();
        // must be ordered!!!
        bool operator==(const keys_pack& other) const;
    };

    struct hotkey
    {
        union
        {
            uintptr_t used;
            void* source;
        };

        int mode; // press/hold etc OR toggle/press
        keys_pack keys;
        bool instant; // if true do call from wndproc
        function_view<void() const> func;
        bool active = false;
    };

    export class context_impl : public basic_context
    {
        imgui_backup backup_;
        ImGuiContext context_;
        ImFontAtlas font_atlas_;
        std::vector<function_view<void() const>> data_;
        std::vector<hotkey> hotkeys_;

      public:
        ~context_impl() override;
        context_impl(void* data, const bool store_settings);

        /*context(const context&)            = delete;
        context& operator=(const context&) = delete;*/

        void release_textures() override;
        void render(void* data) override;
        char process_keys(void* data) override;

        template <typename Fn>
        void store(Fn&& fn)
        {
            data_.emplace_back(std::forward<Fn>(fn));
        }

        void store_hotkey(hotkey&& hk, const bool sort_keys = true);
        void remove_hotkey(void* source);
        void remove_hotkey(const keys_pack& keys);
        void remove_hotkey(void* source, const keys_pack& keys);
    };
} // namespace fd::gui
