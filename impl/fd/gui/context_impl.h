#pragma once

#include <fd/functional.h>
#include <fd/gui/context.h>

#include <imgui_internal.h>

#include <Windows.h>
#include <d3d9.h>

#include <vector>

namespace fd::gui
{
    class imgui_backup
    {
        ImGuiMemAllocFunc allocator_;
        ImGuiMemFreeFunc  deleter_;
        void*             userData_;
        ImGuiContext*     context_;

      public:
        ~imgui_backup();
        imgui_backup();

        imgui_backup(const imgui_backup&)            = delete;
        imgui_backup& operator=(const imgui_backup&) = delete;
    };

    using basic_keys_pack = std::vector<ImGuiKey>;

    class keys_pack : public basic_keys_pack
    {
        string name_;

      public:
        keys_pack() = default;

        using basic_keys_pack::basic_keys_pack;
        using basic_keys_pack::operator=;

        string_view name() const;
        void        update_name();
    };

    using callback_type = function<void() const>;

    struct hotkey_data
    {
        hotkey_source source;
        hotkey_mode   mode;
        callback_type callback;
        hotkey_access access;
        keys_pack     keys;
    };

    struct hotkey : hotkey_data, basic_hotkey
    {
        hotkey() = default;
        hotkey(hotkey_data&& data);

        bool update(bool allowOverride);
        void mark_unused();

      private:
        hotkey_source source() const override;
        hotkey_mode   mode() const override;
        hotkey_access access() const override;
        string_view   name() const override;
        void          callback() const override;
    };

    class hotkeys_storage
    {
        std::vector<hotkey> storage_;

      public:
        using pointer       = hotkey*;
        using const_pointer = const hotkey*;

        pointer       find(hotkey_source source, hotkey_mode mode);
        const_pointer find(hotkey_source source, hotkey_mode mode) const;
        bool          contains(hotkey_source source, hotkey_mode mode) const;
        bool          contains(const hotkey_data& other) const;
        pointer       find_unused();
        void          fire(hotkey_access access);
        pointer       create(hotkey&& hkTmp);
        bool          erase(hotkey_source source, hotkey_mode mode);
    };

    class context final : public basic_context
    {
#ifdef _DEBUG
        imgui_backup backup_;
#endif
        ImGuiContext context_;
        ImFontAtlas  fontAtlas_;

        std::vector<callback_type> callbacks_;
        hotkeys_storage            hotkeys_;

        bool focused_;
        bool hotkeysActive_;

        basic_hotkey* find_basic_hotkey(hotkey_source source, hotkey_mode mode) override;

      public:
        ~context() override;
        context(IDirect3DDevice9* d3d, HWND hwnd, bool storeSettings = false);

        void release_textures() override;
        void render(void* data) override;
        void render(IDirect3DDevice9* thisPtr);

        process_keys_result process_keys(void* data) override;
        process_keys_result process_keys(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

        void store(callback_type&& callback);

        hotkey* create_hotkey(hotkey_data&& hk);
        hotkey* find_hotkey(hotkey_source source, hotkey_mode mode);
        bool    erase_hotkey(hotkey_source source, hotkey_mode mode);
        bool    update_hotkey(hotkey_source source, hotkey_mode mode, bool allowOverride);
    };
} // namespace fd::gui