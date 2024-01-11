#pragma once
#include "type_traits/conditional.h"

#include <Windows.h>

namespace fd::win
{
namespace detail
{
#define RECT_VIEW_CONSTRUCT(_NAME_)             \
    friend struct rect_cpp20;                   \
    friend struct rect_union;                   \
                                                \
    _NAME_() = default;                         \
                                                \
  protected:                                    \
    _NAME_& operator=(_NAME_ const&) = default; \
    _NAME_(_NAME_ const&)            = default;

struct rect_view_x
{
    operator LONG const&() const;
    operator LONG&();

    RECT_VIEW_CONSTRUCT(rect_view_x);
};

struct rect_view_y
{
    operator LONG const&() const;
    operator LONG&();

    RECT_VIEW_CONSTRUCT(rect_view_y);
};

struct rect_view_w
{
    operator LONG() const;

    RECT_VIEW_CONSTRUCT(rect_view_w);
};

struct rect_view_h
{
    operator LONG() const;

    RECT_VIEW_CONSTRUCT(rect_view_h);
};

#undef RECT_VIEW_CONSTRUCT

struct rect_cpp20 : RECT
{
    [[no_unique_address]] rect_view_x x;
    [[no_unique_address]] rect_view_y y;
    [[no_unique_address]] rect_view_w w;
    [[no_unique_address]] rect_view_h h;
};

struct rect_union : RECT
{
    union
    {
        rect_view_x x;
        rect_view_y y;
        rect_view_w w;
        rect_view_h h;
    };
};

struct rect_small
{
    union
    {
        LONG left, x;
    };

    union
    {
        LONG top, y;
    };

    union
    {
        LONG right;
        rect_view_w w;
    };

    union
    {
        LONG bottom;
        rect_view_h h;
    };

    rect_small() = default;
    rect_small(RECT const& r);

    operator RECT const&() const;
    operator RECT&();

    RECT const* operator&() const;
    RECT* operator&();
};
} // namespace detail

using rect = conditional_t<
    sizeof(detail::rect_cpp20) == sizeof(RECT), //
    detail::rect_cpp20,
#ifdef _DEBUG
    detail::rect_union
#else
    detail::rect_small
#endif
    >;

// bool operator==(RECT const& left, RECT const& right) noexcept;
bool operator==(rect const& left, rect const& right) noexcept;

#if 0
struct window_size_simple
{
    LONG w;
    LONG h;

    window_size_simple();
    window_size_simple(RECT const& rect);
    window_size_simple(LONG w, LONG h);

    bool operator==(window_size_simple const& other) const = default;
};

struct window_size : window_size_simple
{
    LONG x;
    LONG y;

    window_size();
    window_size(RECT const& rect);

    window_size& operator=(window_size_simple const& parent_size);
};
#endif

class window_info final
{
    HWND handle_;

  public:
    window_info(HWND handle);

    HWND handle() const;
    WNDPROC proc() const;
    rect size() const;

    bool minimized() const;
};

#if 0
struct window_info_static
{
    union
    {
        HWND handle;
        window_info info;
    };

    WNDPROC proc;
    rect size;
    bool minimized;

    window_info_static(HWND handle);

    void update();
};
#endif
} // namespace fd::win