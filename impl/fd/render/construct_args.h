#pragma once

#include <fd/manual_construct.h>

#include <boost/hana/tuple.hpp>

namespace fd
{
template <typename... T>
using construct_args = boost::hana::tuple<T...>;

template <typename T>
constexpr size_t constuct_args_count = -1;

template <typename... T>
constexpr size_t constuct_args_count<construct_args<T...>> = sizeof...(T);

template <typename T, typename... Args>
void apply_construct_args(T &object, construct_args<Args...> &args)
{
    [&]<size_t I>(std::index_sequence<I...>) {
        construct_manual_object(object, boost::hana::at_c<I>(std::move(args))...);
    }(std::make_index_sequence<sizeof...(Args)>());
}

template <typename T, class A, typename... Args>
void apply_construct_args_front(T &object, A &args, Args &&...extra_args)
{
    [&]<size_t I>(std::index_sequence<I...>) {
        construct_manual_object(object, std::forward<Args>(extra_args)..., boost::hana::at_c<I>(std::move(args))...);
    }(std::make_index_sequence<constuct_args_count<A>>());
}

template <typename T, class A, typename... Args>
void apply_construct_args_back(T &object, A &args, Args &&...extra_args)
{
    [&]<size_t I>(std::index_sequence<I...>) {
        construct_manual_object(object, boost::hana::at_c<I>(std::move(args))..., std::forward<Args>(extra_args)...);
    }(std::make_index_sequence<constuct_args_count<A>>());
}

}