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

    using keys_pack = std::vector<ImGuiKey>;

    using callback_type = function_view<void() const>;

    export enum hotkey_mode
    {
        press,
        held,
        // any
    };

    struct hotkey
    {
        void* source;
        hotkey_mode mode;
        callback_type callback;
        keys_pack keys;
    };

    export class context_impl : public basic_context
    {
        imgui_backup backup_;
        ImGuiContext context_;
        ImFontAtlas font_atlas_;
        std::vector<callback_type> callbacks_;
        std::vector<hotkey> hotkeys_;
        bool focused_;

        void fire_hotkeys();
        bool can_process_keys() const;
        hotkey* find_hotkey(void* source, hotkey_mode mode);
        hotkey* find_unused_hotkey();

      public:
        ~context_impl() override;
        context_impl(void* data, const bool store_settings);

        /*context(const context&)            = delete;
        context& operator=(const context&) = delete;*/

        void release_textures() override;
        void render(void* data) override;
        char process_keys(void* data) override;

        void store(callback_type callback);

        // todo: remove fill_keys
        bool create_hotkey(void* source, hotkey_mode mode, callback_type callback, const bool fill_keys);
        bool update_hotkey(void* source, hotkey_mode mode, const bool allow_override);
        bool remove_hotkey(void* source, hotkey_mode mode);

        bool minimized() const;
    };
} // namespace fd::gui
