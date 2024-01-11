#include "functional/cast.h"
#include "winapi/window_info.h"

namespace fd::win
{
// static_assert(sizeof(rect) == sizeof(RECT));

#if 0
 LONG rect::x() const
{
     return top;
 }

 LONG rect::y() const
{
     return left;
 }

 LONG rect::w() const
{
     return right - left;
 }

 LONG rect::h() const
{
     return bottom - top;
 }


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
#endif

window_info::window_info(HWND handle)
    : handle_{handle}
{
}

namespace detail
{
template <class T>
static auto get_rect_view0(T* ptr, size_t const offset) -> conditional_t<std::is_const_v<T>, RECT const, RECT>*
{
    return unsafe_cast_from(reinterpret_cast<uintptr_t>(ptr) - offset);
}

template <class T>
static auto get_rect_view(T* ptr, size_t const offset)
{
    auto const ret = get_rect_view0(ptr, offset);
    return ret;
}

rect_view_x::operator LONG const&() const
{
    return get_rect_view(this, offsetof(rect, x))->top;
}

rect_view_x::operator LONG&()
{
    return get_rect_view(this, offsetof(rect, x))->top;
}

rect_view_y::operator LONG const&() const
{
    return get_rect_view(this, offsetof(rect, y))->left;
}

rect_view_y::operator LONG&()
{
    return get_rect_view(this, offsetof(rect, y))->left;
}

rect_view_w::operator LONG() const
{
    auto const src = get_rect_view(this, offsetof(rect, w));
    return src->right - src->left;
}

rect_view_h::operator LONG() const
{
    auto const src = get_rect_view(this, offsetof(rect, h));
    return src->bottom - src->top;
}

rect_small::rect_small(RECT const& r)
{
    static_assert(sizeof(rect_small) == sizeof(RECT));
    *reinterpret_cast<RECT*>(this) = r;
}

rect_small::operator tagRECT const&() const
{
    return *reinterpret_cast<RECT const*>(this);
}

rect_small::operator tagRECT&()
{
    return *reinterpret_cast<RECT*>(this);
}

RECT const* rect_small::operator&() const
{
    return reinterpret_cast<RECT const*>(this);
}

RECT* rect_small::operator&()
{
    return reinterpret_cast<RECT*>(this);
}

} // namespace detail

bool operator==(RECT const& left, RECT const& right) noexcept
{
    return left.left == right.left && left.top == right.top && left.right == right.right && left.bottom == right.bottom;
}

bool operator==(rect const& left, rect const& right) noexcept
{
    // ReSharper disable CppRedundantCastExpression
    return static_cast<RECT const&>(left) == static_cast<RECT const&>(right);
    // ReSharper restore CppRedundantCastExpression
}

HWND window_info::handle() const
{
    return handle_;
}

WNDPROC window_info::proc() const
{
    return unsafe_cast_from(GetWindowLongPtr(handle_, GWLP_WNDPROC));
}

rect window_info::size() const
{
    rect r;
    /*GetWindowRect*/ GetClientRect(handle_, &r);
    return r;
}

bool window_info::minimized() const
{
    return IsIconic(handle_);
}

#if 0
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
#endif
} // namespace fd::win