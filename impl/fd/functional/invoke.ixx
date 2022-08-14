module;

#include <function2/function2.hpp>

#include <concepts>
#include <functional>

export module fd.functional.invoke;

export namespace fd
{
    // using std::invocable;
    using fu2::detail::invocation::invoke;
    using std::invoke;

    template <typename T, typename... Args>
    concept invocable = requires(T&& obj, Args&&... args)
    {
        invoke(std::forward<T>(obj), std::forward<Args>(args)...);
    };
} // namespace fd
