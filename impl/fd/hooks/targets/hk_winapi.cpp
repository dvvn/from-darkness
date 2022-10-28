module;

#include <fd/assert.h>

#ifdef IMGUI_USER_CONFIG
#include IMGUI_USER_CONFIG
#else
#include <imgui_internal.h>
#endif

#include <windows.h>

module fd.hooks.winapi;
import fd.gui.context;
#ifdef IMGUI_DISABLE_DEMO_WINDOWS
import fd.gui.menu;
#endif

// #define HOT_UNLOAD_SUPPORTED

using namespace fd;
using namespace hooks;

static wndproc* _Wndproc;

wndproc::wndproc(HWND hwnd, WNDPROC target)
{
    this->init(target, &wndproc::callback);
    this->def_ = IsWindowUnicode(hwnd) ? DefWindowProcW : DefWindowProcA;
#ifdef _DEBUG
    this->hwnd_ = hwnd;
#endif
    _Wndproc = this;
}

wndproc::~wndproc()
{
    // added for logging only
    if (*this)
        impl::disable();
}

wndproc::wndproc(wndproc&& other)
    : impl(std::move(other))
{
    def_ = other.def_;
#ifdef _DEBUG
    hwnd_ = other.hwnd_;
#endif
    _Wndproc = this;
}

string_view wndproc::name() const
{
    return "WinAPI.WndProc";
}

LRESULT WINAPI wndproc::callback(wndproc_data data)
{
    FD_ASSERT(data.window == _Wndproc->hwnd_);

#ifdef IMGUI_DISABLE_DEMO_WINDOWS
    if (!gui::menu->visible())
        return invoke(&wndproc::callback, _Wndproc->get_original_method(), data);
#endif

    switch (gui::context->process_keys(&data))
    {
    case TRUE:
        return TRUE;
    case FALSE:
        return invoke(&wndproc::callback, _Wndproc->get_original_method(), data);
    default:
        return invoke(&wndproc::callback, static_cast<void*>(_Wndproc->def_), data);
    }
}
