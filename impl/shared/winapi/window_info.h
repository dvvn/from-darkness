#pragma once

#include <Windows.h>

namespace fd::win
{
struct window_size_simple
{
    LONG w;
    LONG h;

    window_size_simple();
    window_size_simple(RECT const& rect);
    window_size_simple(LONG w, LONG h);

    bool operator==(window_size_simple const& other) const;
};

struct window_size : window_size_simple
{
    LONG x;
    LONG y;

    window_size();
    window_size(RECT const& rect);

    window_size& operator=(window_size_simple const& parent_size);
};

class window_info final
{
    HWND handle_;

  public:
    window_info(HWND handle);

    HWND handle() const;
    WNDPROC proc() const;
    window_size size() const;
    bool minimized() const;
};

struct window_info_static
{
    HWND handle;
    WNDPROC proc;
    window_size size;
    bool minimized;

    window_info_static(HWND handle);

    void update();
};
}