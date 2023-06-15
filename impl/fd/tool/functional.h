#pragma once

#include <boost/hana/functional.hpp>
#include <boost/lambda2.hpp>

#include <functional>

template <typename T>
requires(std::is_member_pointer_v<T>)
struct boost::hana::overload_t<T> : decltype(std::bind_front(std::declval<T>()))
{
    using type = overload_t;
    using base = decltype(std::bind_front(std::declval<T>()));

    using base::base;
};

namespace fd
{
namespace placeholders = boost::lambda2;

using std::bind;
using std::bind_back;
using std::bind_front;

using boost::hana::overload;

// template <typename Fn>
// struct function_return : function_return<decltype(std::function(std::declval<std::decay_t<Fn>>()))>
//{
// };
//
// template <typename Ret, typename... Args>
// struct function_return<std::function<Ret(Args...)>>
//{
//     using type = Ret;
// };
//
// template <typename Ret, typename... Fn>
// constexpr auto overload_r(Fn &&...fn)
//{
//     auto try_wrap = []<typename T>(T &&func) -> decltype(auto) {
//         using this_ret = typename function_return<T>::type;
//         if constexpr (std::same_as<this_ret, Ret>)
//             return std::forward<T>(func);
//         else
//             return [stored = std::forward<T>(func)]<typename... Args>(Args &&...args) mutable -> Ret {
//                 return std::invoke_r<Ret>(stored, std::forward<Args>(args)...);
//             };
//     };
//
//     return overload(try_wrap(std::forward<Fn>(fn))...);
// }
//
// template <typename Fn, typename... Other>
// constexpr auto overload_r(Fn &&fn, Other &&...other)
//{
//     using first_ret = typename function_return<Fn>::type;
//     return overload_r<first_ret>(std::forward<Fn>(fn), std::forward<Other>(other));
// }

}