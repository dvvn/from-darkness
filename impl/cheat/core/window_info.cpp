module;

#include <cheat/core/object.h>

#include <d3d9.h>

#include <windows.h>

#include <utility>

module cheat.tools.window_info;

using namespace cheat;

static auto _Get_focused_window() noexcept
{
    D3DDEVICE_CREATION_PARAMETERS params;
    // instance_of<IDirect3DDevice9>->GetCreationParameters(&params);
    return params.hFocusWindow;
}

std::pair<size_t, size_t> tools::window_size() noexcept
{
    RECT rect;
    /*GetWindowRect*/ GetClientRect(_Get_focused_window(), &rect);
    const size_t width = rect.right - rect.left;
    const size_t height = rect.bottom - rect.top;

    return {width, height};
}

std::pair<void*, long> tools::window_proc() noexcept
{
    const auto hwnd = _Get_focused_window();
    const auto unicode = IsWindowUnicode(hwnd);

    void* const def = unicode ? DefWindowProcW : DefWindowProcA;
    const long curr = std::invoke(unicode ? GetWindowLongPtrW : GetWindowLongPtrA, hwnd, GWLP_WNDPROC);

    return {def, curr};
}
