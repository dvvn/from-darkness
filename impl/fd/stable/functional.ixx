module;

#include <function2/function2.hpp>

#include <concepts>
#include <functional>

export module fd.functional;

// using std::invocable;
using fu2::detail::invocation::invoke;
using std::invoke;

template <typename T, typename... Args>
concept invocable = requires(T&& obj, Args&&... args)
{
    invoke(std::forward<T>(obj), std::forward<Args>(args)...);
};

/* template <typename Fn>
struct function : std::function<Fn>
{
    using std::function<Fn>::function;
};
using std::function;
*/

enum class bind_mode : uint8_t
{
    front,
    back
};

template <bind_mode Mode>
struct bind_caller;

template <>
struct bind_caller<bind_mode::front>
{
    template <typename Fn, class Tpl, typename... Args>
    static constexpr decltype(auto) call(Fn& fn, Tpl& bound_args_stored, Args&&... call_args)
    {
        if constexpr (std::tuple_size_v<Tpl> == 0)
            return invoke(fn, std::forward<Args>(call_args)...);
        else
        {
            return std::apply(
                [&](auto&... bound_args) -> decltype(auto) {
                    return invoke(fn, bound_args..., std::forward<Args>(call_args)...);
                },
                bound_args_stored);
        }
    }
};

template <>
struct bind_caller<bind_mode::back>
{
    template <typename Fn, class Tpl, typename... Args>
    static constexpr decltype(auto) call(Fn& fn, Tpl& bound_args_stored, Args&&... call_args)
    {
        if constexpr (std::tuple_size_v<Tpl> == 0)
            return invoke(fn, std::forward<Args>(call_args)...);
        else
        {
            return std::apply(
                [&](auto&... bound_args) -> decltype(auto) {
                    return invoke(fn, std::forward<Args>(call_args)..., bound_args...);
                },
                bound_args_stored);
        }
    }
};

template <bind_mode Mode>
struct _Bind_helper
{
    template <typename Fn, typename... Args>
    constexpr auto operator()(Fn&& fn, Args&&... args) const
    {
        return [fn_stored         = std::forward<Fn>(fn),
                bound_args_stored = std::forward_as_tuple(std::forward<Args>(args)...)]<typename... CallArgs>(CallArgs&&... call_args) -> decltype(auto) {
            return bind_caller<Mode>::call(fn_stored, bound_args_stored, std::forward<CallArgs>(call_args)...);
        };
    }
};

#if defined(__cpp_lib_bind_front)
using std::bind_front;
#else
constexpr _Bind_helper<bind_mode::front> bind_front;
#endif

#if defined(__cpp_lib_bind_back)
using std::bind_back;
#else
constexpr _Bind_helper<bind_mode::back> bind_back;
#endif

export namespace fd
{
    using ::invocable;
    using ::invoke;

    using fu2::function;
    using fu2::function_view;
    using fu2::unique_function;

    using ::bind_back;
    using ::bind_front;
} // namespace fd
