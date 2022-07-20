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

template <typename Fn, typename... Args>
constexpr auto bind_front(Fn&& fn, Args&&... args)
{
#if defined(__cpp_lib_bind_front)
    return std::bind_front(std::forward<Fn>(fn), std::forward<Args>(args)...);
#else
    return [fn_stored = std::forward<Fn>(fn), args_stored = std::tuple(std::forward<Args>(args)...)]<typename... ExtraArgs>(ExtraArgs&&... extra_args) -> decltype(auto) {
        return std::apply(
            [&](auto&&... args_unpacked) -> decltype(auto) {
                return invoke(fn_stored, args_unpacked..., std::forward<ExtraArgs>(extra_args)...);
            },
            args_stored);
    };
#endif
}

template <typename Fn, typename... Args>
constexpr auto bind_back(Fn&& fn, Args&&... args)
{
#ifdef __cpp_lib_bind_back
    return std::bind_back(std::forward<Fn>(fn), std::forward<Args>(args)...);
#else
    static_assert(std::_Always_false<Fn>, "Not implemented");
#endif
}

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
