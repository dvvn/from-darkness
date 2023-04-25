#pragma once

#include <fd/gui/basic_context.h>

#if !defined(IMGUI_VERSION) && defined(IMGUI_USER_CONFIG)
#include IMGUI_USER_CONFIG
#endif
#include <imgui_internal.h>
//

#include <Windows.h>
#include <d3d9.h>

#include <utility>

namespace fd
{
class imgui_backup
{
    ImGuiMemAllocFunc allocator_;
    ImGuiMemFreeFunc deleter_;
    void *userData_;
    ImGuiContext *context_;

  public:
    ~imgui_backup();
    imgui_backup();

    imgui_backup(imgui_backup const &)            = delete;
    imgui_backup &operator=(imgui_backup const &) = delete;
};

class _gui_context : public basic_gui_context
{
    // demo windows have static ImVector inside, so we unable to restore allocators here
#if defined(IMGUI_DISABLE_DEMO_WINDOW) && defined(_DEBUG)
    imgui_backup backup_;
#endif
    ImGuiContext context_;
    ImFontAtlas font_atlas_;

    bool attached_;

  public:
    struct init_data
    {
        bool store_settings;
        IDirect3DDevice9 *backend;
        HWND window;
    };

    ~_gui_context() override;
    _gui_context();

    bool init(init_data data);

    _gui_context(_gui_context const &other)            = delete;
    _gui_context &operator=(_gui_context const &other) = delete;

    // mark whe rendered unavaiable
    void detach();

    void release_textures() override;

  private:
    keys_return process_keys(void *data) override;

  protected:
    bool begin_frame();
    void end_frame(IDirect3DDevice9 *thisPtr);

  public:
    keys_return process_keys(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

#if 0
    LRESULT process_keys(auto orig, WNDPROC def, auto... args)
    {
        switch (process_keys(args...))
        {
        case keys_return::instant:
            return TRUE;
        case keys_return::native:
            return orig(args...);
        case keys_return::def:
            return def(args...);
        default:
            std::unreachable();
        }
    }
#endif
};

template <typename Callback>
class gui_context final : public _gui_context
{
    Callback callback_;

  public:
    gui_context(Callback callback)
        : callback_(std::move(callback))
    {
    }

  private:
    void render(void *data) override
    {
        render(static_cast<IDirect3DDevice9 *>(data));
    }

  public:
    void render(IDirect3DDevice9 *thisPtr)
    {
        if (begin_frame())
        {
            callback_();
            end_frame(thisPtr);
        }
    }
};

} // namespace fd