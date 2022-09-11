module;

#include <functional>
#include <limits>
#include <tuple>

export module fd.functional.bind;
export import fd.functional.invoke;

using fd::invoke;

enum class bind_mode : uint8_t
{
    front,
    back
};

template <bind_mode Mode>
struct bind_caller;

#ifndef __cpp_lib_bind_front
template <>
struct bind_caller<bind_mode::front>
{
    template <typename Fn, class Tpl, typename... Args>
    static constexpr decltype(auto) call(Fn& fn, Tpl& bound_args_stored, Args&&... call_args)
    {
        return std::apply(
            [&](auto&... bound_args) -> decltype(auto) {
                return invoke(fn, bound_args..., std::forward<Args>(call_args)...);
            },
            bound_args_stored);
    }
};
#endif

#ifndef __cpp_lib_bind_back
template <>
struct bind_caller<bind_mode::back>
{
    template <typename Fn, class Tpl, typename... Args>
    static constexpr decltype(auto) call(Fn& fn, Tpl& bound_args_stored, Args&&... call_args)
    {
        return std::apply(
            [&](auto&... bound_args) -> decltype(auto) {
                return invoke(fn, std::forward<Args>(call_args)..., bound_args...);
            },
            bound_args_stored);
    }
};
#endif

template <bind_mode Mode>
struct _Bind_helper
{
    template <typename Fn, typename... Args>
    constexpr auto operator()(Fn&& fn, Args&&... args) const
    {
        return [fn_stored = std::forward<Fn>(fn), bound_args_stored = std::tuple(std::forward<Args>(args)...)]<typename... CallArgs>(CallArgs&&... call_args) -> decltype(auto) {
            if constexpr (sizeof...(Args) == 0)
                return invoke(fn_stored, std::forward<CallArgs>(call_args)...);
            else
                return bind_caller<Mode>::call(fn_stored, bound_args_stored, std::forward<CallArgs>(call_args)...);
        };
    }
};

export namespace fd
{
#ifdef __cpp_lib_bind_front
    using std::bind_front;
#else
    constexpr _Bind_helper<bind_mode::front> bind_front;
#endif

#ifdef __cpp_lib_bind_back
    using std::bind_back;
#else
    constexpr _Bind_helper<bind_mode::back> bind_back;
#endif

} // namespace fd
