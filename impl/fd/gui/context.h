#pragma once

#include <fd/gui/basic_context.h>

#include <imgui_internal.h>

#include <Windows.h>
#include <d3d9.h>

#include <utility>

namespace fd
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

    imgui_backup(imgui_backup const&)            = delete;
    imgui_backup& operator=(imgui_backup const&) = delete;
};

class _gui_context : public basic_gui_context
{
#ifdef _DEBUG
    imgui_backup backup_;
#endif
    ImGuiContext context_;
    ImFontAtlas  fontAtlas_;

    bool focused_;
    bool attached_;

  public:
    ~_gui_context() override;
    _gui_context();

    struct init_data
    {
        bool              storeSettings;
        IDirect3DDevice9* backend;
        HWND              window;
    };

    [[nodiscard]]
    bool init(init_data initData);

    // mark whe rendered unavaiable
    void detach();

    void release_textures() override;

  private:
    keys_return process_keys(void* data) override;

  protected:
    bool begin_frame();
    void end_frame(IDirect3DDevice9* thisPtr);

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
        : _gui_context()
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

} // namespace fd