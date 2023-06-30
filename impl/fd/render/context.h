#pragma once

#include "basic_context.h"
#include "own_backend.h"

#include "fd/vfunc.h"

namespace fd
{
class system_library_info;

class internal_render_backend
{
    IDirect3DDevice9 **device_;

  public:
    internal_render_backend(system_library_info info);

    IDirect3DDevice9 *backend() const;
    HWND window() const;
    WNDPROC window_proc() const;
    WNDPROC default_window_proc() const;
};

template <bool External>
struct render_context;

namespace detail
{
template <bool External>
struct render_vfunc_accesser
{
    using base = render_context<External>;

    template <typename Ret, typename... Args>
    using vfunc_t = vfunc<call_type_t::stdcall_, Ret, IDirect3DDevice9, Args...>;

    template <typename Ret, typename... Args>
    vfunc_t<Ret, Args...> operator[](Ret (__stdcall IDirect3DDevice9::*func)(Args...)) const
    {
        return {func, static_cast<base const *>(this)->backend()};
    }
};
} // namespace detail

template <>
struct render_context<true> : own_render_backend, detail::render_vfunc_accesser<true>, basic_render_context
{
    render_context();
};

template <>
struct render_context<false> : internal_render_backend, detail::render_vfunc_accesser<false>, basic_render_context
{
    ~render_context();
    render_context(system_library_info info);
};
} // namespace fd