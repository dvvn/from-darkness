module;

#include <function2/function2.hpp>

export module fd.functional.overload;
export import fd.functional.invoke;

export namespace fd
{
    using fu2::detail::overloading::overload;
    using fu2::detail::overloading::overload_impl;

    /* template <typename... Ts>
    overload_impl(Ts&&...) -> overload_impl<std::decay_t<Ts>...>; */

} // namespace fd
