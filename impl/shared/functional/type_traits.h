#pragma once

#include <functional>

namespace fd
{
// template <typename Fn>
// struct function_argument : function_argument<decltype(std::function(std::declval<Fn>()))>
//{
// };

template <bool, size_t I, typename T, typename... Ts>
struct argument;

template <size_t I, typename T, typename... Ts>
struct argument<true, I, T, Ts...> : argument<true, I - 1, Ts...>
{
    using type = T;
};

template <typename T, typename... Ts>
struct argument<true, 0, T, Ts...>
{
    using type = T;
};

template <size_t I, typename T, typename... Ts>
struct argument<false, I, T, Ts...>
{
    using type = void;
};

//---

template <size_t I, typename... Ts>
using argument_t = typename argument<(I < sizeof...(Ts)), I, Ts...>::type;

template <size_t I, typename Fn>
struct function_argument;

template <size_t I, typename Fn>
struct function_argument<I, std::reference_wrapper<Fn>> : function_argument<I, Fn>
{
};

//[[deprecated]]
template <size_t I, typename Ret, typename... Args>
struct function_argument<I, std::function<Ret(Args...)>>
{
    using type = argument_t<I, Args...>;
};
} // namespace fd
