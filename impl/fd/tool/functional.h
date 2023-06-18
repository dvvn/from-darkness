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

//template <typename Fn>
//FD_WRAP_TOOL(function, std::function<Fn>);
}