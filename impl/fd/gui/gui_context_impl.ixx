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

    export enum render_result
    {
        skipped       = 0,
        visible       = 1 << 0,
        input         = 1 << 1,
        visible_input = visible | input
    };

    using callback_type = function_view<render_result() const>;

    export enum hotkey_mode
    {
        press,
        held,
        // any
    };

    using hotkey_source = uint8_t;

    struct hotkey
    {
        hotkey_source source;
        hotkey_mode mode;
        callback_type callback;
        keys_pack keys;

        bool update(const bool allow_override);
    };

    class hotkeys_storage
    {
        std::vector<hotkey> storage_;

      public:
        using pointer       = hotkey*;
        using const_pointer = const hotkey*;

        pointer find(hotkey_source source, hotkey_mode mode);
        const_pointer find(hotkey_source source, hotkey_mode mode) const;
        bool contains(hotkey_source source, hotkey_mode mode) const;
        pointer find_unused();
        void fire();
        hotkey* create(hotkey_source source, hotkey_mode mode, callback_type callback);
    };

    export class context_impl : public basic_context
    {
        imgui_backup backup_;
        ImGuiContext context_;
        ImFontAtlas font_atlas_;
        std::vector<callback_type> callbacks_;
        hotkeys_storage hotkeys_;
        bool focused_;

        void fire_hotkeys();
        bool can_process_keys() const;

      public:
        ~context_impl() override;
        context_impl(void* data, const bool store_settings);

        /*context(const context&)            = delete;
        context& operator=(const context&) = delete;*/

        void release_textures() override;
        void render(void* data) override;
        char process_keys(void* data) override;

        void store(callback_type callback);

        bool create_hotkey(hotkey_source source, hotkey_mode mode, callback_type callback, const bool update = false);
        bool update_hotkey(hotkey_source source, hotkey_mode mode, const bool allow_override);
        bool remove_hotkey(hotkey_source source, hotkey_mode mode);
        bool contains_hotkey(hotkey_source source, hotkey_mode mode) const;

        bool minimized() const;
    };

} // namespace fd::gui
