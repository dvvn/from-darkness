#include "functional/cast.h"
#include "winapi/window_info.h"

namespace fd::win
{
window_size_simple::window_size_simple()
    : w{CW_USEDEFAULT}
    , h{CW_USEDEFAULT}
{
}

window_size_simple::window_size_simple(RECT const& rect)
    : w{rect.right - rect.left}
    , h{rect.bottom - rect.top}
{
}

window_size_simple::window_size_simple(LONG const w, LONG const h)
    : w{w}
    , h{h}
{
}

window_size::window_size()
    : x{CW_USEDEFAULT}
    , y{CW_USEDEFAULT}
{
}

window_size::window_size(RECT const& rect)
    : window_size_simple{rect}
    , x{rect.top}
    , y{rect.left}
{
}

window_size& window_size::operator=(window_size_simple const& parent_size)
{
    window_size_simple::operator=(parent_size);
    return *this;
}

window_info::window_info(HWND handle)
    : handle_{handle}
{
}

HWND window_info::handle() const
{
    return handle_;
}

WNDPROC window_info::proc() const
{
    return unsafe_cast_from(GetWindowLongPtr(handle_, GWLP_WNDPROC));
}

window_size window_info::size() const
{
    RECT rect;
    /*GetWindowRect*/ GetClientRect(handle_, &rect);
    return rect;
}

bool window_info::minimized() const
{
    return IsIconic(handle_);
}

window_info_static::window_info_static(HWND handle)
    : handle{handle}
{
#ifdef _DEBUG
    static_assert(sizeof(window_info) == sizeof(HWND));
#endif
    update();
}

void window_info_static::update()
{
    proc      = info.proc();
    size      = info.size();
    minimized = info.minimized();
}
} // namespace fd::win