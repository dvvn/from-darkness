#pragma once

#include "functional/cast.h"

#include <Windows.h>

namespace fd::win
{
struct window_size_simple
{
    LONG w;
    LONG h;

    window_size_simple()
        : w{CW_USEDEFAULT}
        , h{CW_USEDEFAULT}
    {
    }

    window_size_simple(RECT const& rect)
        : w{rect.right - rect.left}
        , h{rect.bottom - rect.top}
    {
    }

    window_size_simple(LONG const w, LONG const h)
        : w{w}
        , h{h}
    {
    }

    bool operator==(window_size_simple const& other) const = default;
};

struct window_size : window_size_simple
{
    LONG x;
    LONG y;

    window_size()
        : x{CW_USEDEFAULT}
        , y{CW_USEDEFAULT}
    {
    }

    window_size(RECT const& rect)
        : window_size_simple{rect}
        , x{rect.top}
        , y{rect.left}
    {
    }

    window_size& operator=(window_size_simple const& parent_size)
    {
        window_size_simple::operator=(parent_size);
        return *this;
    }
};

class window_info final
{
    HWND handle_;

  public:
    window_info(HWND handle)
        : handle_{handle}
    {
    }

    HWND handle() const
    {
        return handle_;
    }

    WNDPROC proc() const
    {
        return unsafe_cast_from(GetWindowLongPtr(handle_, GWLP_WNDPROC));
    }

    window_size size() const
    {
        RECT rect;
        /*GetWindowRect*/ GetClientRect(handle_, &rect);
        return rect;
    }

    bool minimized() const
    {
        return IsIconic(handle_);
    }
};

struct window_info_static
{
    union
    {
        HWND handle;
        window_info info;
    };

    WNDPROC proc;
    window_size size;
    bool minimized;

    window_info_static(HWND handle)
        : handle{handle}
    {
#ifdef _DEBUG
        static_assert(sizeof(window_info) == sizeof(HWND));
#endif
        update();
    }

    void update()
    {
        proc      = info.proc();
        size      = info.size();
        minimized = info.minimized();
    }
};
} // namespace fd::win