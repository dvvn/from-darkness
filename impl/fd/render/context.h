#pragma once

#include "basic_context.h"
#include "own_backend.h"

#include "fd/vfunc.h"

namespace fd
{
class system_library_info;

namespace detail
{
template <typename T>
class internal_render_backend;

template <>
class internal_render_backend<IDirect3DDevice9>
{
    IDirect3DDevice9 **device_;

  public:
    internal_render_backend(system_library_info info);

    IDirect3DDevice9 *backend() const;
    HWND window() const;
    WNDPROC window_proc() const;
    WNDPROC default_window_proc() const;
};

template <typename T>
struct render_vfunc_accesser
{
    using device_type = FD_RENDER_BACKEND;//std::remove_pointer_t<decltype(std::declval<T>().backend())>;

    template <typename Ret, typename... Args>
    using vfunc_t = vfunc<call_type_t::stdcall_, Ret, device_type, Args...>;

    template <typename Ret, typename... Args>
    vfunc_t<Ret, Args...> operator[](Ret (__stdcall device_type::*func)(Args...)) const
    {
        return {func, static_cast<T const *>(this)->backend()};
    }
};

} // namespace detail

template <bool /*external*/>
struct render_context;

template <>
struct render_context<true> : own_render_backend, //
                              basic_render_context,
                              detail::render_vfunc_accesser<render_context<true>>
{
    render_context();
};

template <>
struct render_context<false> : detail::internal_render_backend<FD_RENDER_BACKEND>,
                               basic_render_context,
                               detail::render_vfunc_accesser<render_context<false>>
{
    ~render_context();
    render_context(system_library_info info);
};
} // namespace fd