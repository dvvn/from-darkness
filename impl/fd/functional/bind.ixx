module;

#include <functional>
#include <limits>
#include <tuple>

export module fd.functional.bind;
export import fd.functional.invoke;

namespace fd
{
    enum class mode : uint8_t
    {
        front,
        back
    };

    template <mode Mode>
    struct bind_caller;

    template <mode Mode>
    struct bind_impl
    {
        template <typename Fn, typename... Args>
        constexpr decltype(auto) operator()(Fn&& fn, Args&&... args) const
        {
            return [_Fn   = std::forward<Fn>(fn), //
                    _Args = std::tuple(std::forward<Args>(args)...)]<typename... CallArgs>(CallArgs&&... call_args) -> decltype(auto) {
                if constexpr (sizeof...(Args) == 0)
                    return invoke(_Fn, std::forward<CallArgs>(call_args)...);
                else
                    return bind_caller<Mode>::call(_Fn, _Args, std::forward<CallArgs>(call_args)...);
            };
        }
    };

#ifdef __cpp_lib_bind_front
    export using std::bind_front;
#else
    template <>
    struct bind_caller<mode::front>
    {
        template <typename Fn, class Tpl, typename... Args>
        static constexpr decltype(auto) call(Fn& fn, Tpl& args, Args&&... call_args)
        {
            return std::apply(
                [&](auto&... bound_args) -> decltype(auto) {
                    return invoke(fn, bound_args..., std::forward<Args>(call_args)...);
                },
                args);
        }
    };

    export constexpr bind_impl<mode::front> bind_front;
#endif

#ifdef __cpp_lib_bind_back
    export using std::bind_back;
#else
    template <>
    struct bind_caller<mode::back>
    {
        template <typename Fn, class Tpl, typename... Args>
        static constexpr decltype(auto) call(Fn& fn, Tpl& args, Args&&... call_args)
        {
            return std::apply(
                [&](auto&... bound_args) -> decltype(auto) {
                    return invoke(fn, std::forward<Args>(call_args)..., bound_args...);
                },
                args);
        }
    };

    export constexpr bind_impl<mode::back> bind_back;
#endif

} // namespace fd
