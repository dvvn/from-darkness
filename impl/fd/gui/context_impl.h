#pragma once

#include <fd/functional.h>
#include <fd/gui/context.h>

#include <imgui_internal.h>

#include <vector>

namespace fd::gui
{
    class imgui_backup
    {
        ImGuiMemAllocFunc allocator_;
        ImGuiMemFreeFunc deleter_;
        void* userData_;
        ImGuiContext* context_;

      public:
        ~imgui_backup();
        imgui_backup();

        imgui_backup(const imgui_backup&)            = delete;
        imgui_backup& operator=(const imgui_backup&) = delete;
    };

    using keys_pack = std::vector<ImGuiKey>;

    using callback_type = function_view<void() const>;

    enum hotkey_mode
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

        bool update(bool allowOverride);
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

    class context_impl final : public basic_context
    {
        imgui_backup backup_;
        ImGuiContext context_;
        ImFontAtlas fontAtlas_;
        std::vector<callback_type> callbacks_;
        hotkeys_storage hotkeys_;
        bool focused_;

        void fire_hotkeys();
        bool can_process_keys() const;

      public:
        ~context_impl() override;
        context_impl(void* data, bool storeSettings);

        /*context(const context&)            = delete;
        context& operator=(const context&) = delete;*/

        void release_textures() override;
        void render(void* data) override;
        char process_keys(void* data) override;

        void store(callback_type callback);

        bool create_hotkey(hotkey_source source, hotkey_mode mode, callback_type callback, bool update = false);
        bool update_hotkey(hotkey_source source, hotkey_mode mode, bool allowOverride);
        bool remove_hotkey(hotkey_source source, hotkey_mode mode);
        bool contains_hotkey(hotkey_source source, hotkey_mode mode) const;
    };
} // namespace fd::gui
