#include "tier1/winapi/window_info.h"

#include <utility>

namespace FD_TIER(1)::win
{
window_size_simple::window_size_simple()
    : w(CW_USEDEFAULT)
    , h(CW_USEDEFAULT)
{
}

window_size_simple::window_size_simple(RECT const& rect)
    : w(rect.right - rect.left)
    , h(rect.bottom - rect.top)
{
}

window_size_simple::window_size_simple(LONG w, LONG h)
    : w(w)
    , h(h)
{
}

bool window_size_simple::operator==(window_size_simple const& other) const
{
#if LONG_MAX == INT_MAX
    if constexpr (sizeof(window_size_simple) == sizeof(uint64_t))
        return *reinterpret_cast<uint64_t const*>(this) == reinterpret_cast<uint64_t const&>(other);
    else
#endif
        return w == other.w && h == other.h;
}

window_size::window_size()
    : x(CW_USEDEFAULT)
    , y(CW_USEDEFAULT)
{
}

window_size::window_size(RECT const& rect)
    : window_size_simple(rect)
    , x(rect.top)
    , y(rect.left)
{
}

window_size& window_size::operator=(window_size_simple const& parent_size)
{
    window_size_simple::operator=(parent_size);
    return *this;
}

window_info::window_info(HWND handle)
    : handle_(handle)
{
}

HWND window_info::handle() const
{
    return handle_;
}

WNDPROC window_info::proc() const
{
    return reinterpret_cast<WNDPROC>(GetWindowLongPtr(handle_, GWLP_WNDPROC));
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
    : handle(handle)
{
    update();
}

void window_info_static::update()
{
    window_info const info(handle);
    proc      = info.proc();
    size      = info.size();
    minimized = info.minimized();
}
}