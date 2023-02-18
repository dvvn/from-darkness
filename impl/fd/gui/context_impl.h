#pragma once

#include <fd/gui/context.h>

#include <imgui_internal.h>

#include <Windows.h>
#include <d3d9.h>

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

#ifdef FD_HAVE_HOTKEY

using keys_pack_view = std::span<const ImGuiKey>;

class keys_pack : public std::vector<ImGuiKey>
{
    string name_;

  public:
    keys_pack() = default;
    keys_pack(std::initializer_list<value_type> list);

    string_view name() const;
    void        update_name();
};

using hotkey_callback = function<void() const>;

template <class K>
struct basic_hotkey_info
{
    hotkey_mode mode;
    K           keys;

    basic_hotkey_info() = default;

    template <typename K2 = K>
    basic_hotkey_info(const basic_hotkey_info<K2>& other)
        : mode(other.mode)
        , keys(other.keys)
    {
    }

    basic_hotkey_info(basic_hotkey_info&& other)
        : mode(std::move(other.mode))
        , keys(std::move(other.keys))
    {
    }

    template <class K2 = K>
    bool operator==(const basic_hotkey_info<K2>& other) const
    {
        return mode == other.mode && keys == other.keys;
    }
};

template <class K>
struct basic_hotkey_info_ex : basic_hotkey_info<K>
{
    hotkey_access access;

    basic_hotkey_info_ex() = default;

    template <typename K2 = K>
    basic_hotkey_info_ex(const basic_hotkey_info_ex<K2>& other)
        : basic_hotkey_info<K>(other)
        , access(other.access)
    {
    }

    template <typename K2 = K>
    basic_hotkey_info_ex(basic_hotkey_info_ex<K2>&& other)
        : basic_hotkey_info<K>(std::move(other))
        , access(std::move(other.access))
    {
    }

    basic_hotkey_info_ex(hotkey_mode mode, K keys, hotkey_access access)
        : basic_hotkey_info<K>{ mode, keys }
        , access(access)
    {
    }

    template <class K2 = K>
    bool operator==(const basic_hotkey_info_ex<K2>& other) const
    {
        if (access == other.access || access == any || other.access == any)
            return basic_hotkey_info<K>::operator==(other);
        return false;
    }
};

struct hotkey_info : basic_hotkey_info_ex<keys_pack>
{
    using basic_hotkey_info_ex::basic_hotkey_info_ex;

    bool update(bool allowOverride);
};

struct hotkey_info_view : basic_hotkey_info_ex<keys_pack_view>
{
    using basic_hotkey_info_ex::basic_hotkey_info_ex;
};

struct hotkey_data
{
    hotkey_callback callback;
    // reserved
};

class hotkey : public basic_hotkey
{
    using value_type = std::pair<hotkey_info, hotkey_data>;

    std::vector<value_type> storage_;
    string                  mergedName_;

  public:
    hotkey_data& operator[](hotkey_info&& info);
    void         store(hotkey_info&& info, hotkey_data&& data);

    const hotkey_data& operator[](const hotkey_info_view& info) const;
    bool               contains(const hotkey_info_view& info) const;

    bool update(bool allowOverride);
    bool update(hotkey_mode mode, bool allowOverride);
    void update_name();

    void fire(hotkey_access access);

    //---

    string_view name() const override;
    void        callback() const override;
};

struct hotkeys_storage : unordered_map<hotkey_source, hotkey>
{
    void fire(hotkey_access access);
};

#endif

class context_impl : public basic_context
{
#ifdef _DEBUG
    imgui_backup backup_;
#endif
    ImGuiContext context_;
    ImFontAtlas  fontAtlas_;

#ifdef FD_HAVE_HOTKEY
    hotkeys_storage hotkeys_;
    bool            hotkeysActive_;
#endif

    bool focused_;

  public:
    ~context_impl() override;
    context_impl();

    [[nodiscard]]
    bool init(bool storeSettings = false);
    [[nodiscard]]
    bool init(IDirect3DDevice9* d3d);
    [[nodiscard]]
    bool init(HWND hwnd);

    void release_textures() override;

  private:
    process_keys_result process_keys(void* data) override;

  protected:
    bool begin_frame();
    void end_frame(IDirect3DDevice9* thisPtr);

  public:
    process_keys_result process_keys(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

#ifdef FD_HAVE_HOTKEY
    void          create_hotkey(hotkey_source source, hotkey_info&& info, hotkey_data&& data);
    hotkey&       get_hotkey(hotkey_source source);
    // bool    erase_hotkey(hotkey_source source, hotkey_mode mode);
    bool          update_hotkey(hotkey_source source, hotkey_mode mode, bool allowOverride);
    basic_hotkey* find_hotkey(hotkey_source source) override;
#endif
};

template <typename Callback>
class context final : public context_impl
{
    Callback callback_;

  public:
    context(Callback callback)
        : context_impl()
        , callback_(std::move(callback))
    {
    }

  private:
    void render(void* data) override
    {
        render(static_cast<IDirect3DDevice9*>(data));
    }

  public:
    void render(IDirect3DDevice9* thisPtr)
    {
        if (begin_frame())
        {
            callback_();
            end_frame(thisPtr);
        }
    }
};

} // namespace fd::gui