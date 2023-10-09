#pragma once

#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

#include <concepts>

namespace fd
{
template <class... T>
using menu_item_getter = boost::hana::tuple<T...>;

template <class... T, typename Fn>
void apply(menu_item_getter<T...> const& items, Fn invoker)
{
    if constexpr ((std::invocable<Fn, T&> && ...))
        boost::hana::for_each(items, invoker);
    else
        boost::hana::for_each(items, [&invoker]<class C>(C& value) {
            if constexpr (std::is_pointer_v<C> && !std::invocable<Fn, C>)
                std::invoke(invoker, *value);
            else
                std::invoke(invoker, value);
        });
}

template <class... T>
void apply(menu_item_getter<T...> const& items)
{
    boost::hana::for_each(items, []<class C>(C& item) {
        using raw_t = std::remove_pointer_t<C>;
        std::invoke(&raw_t::render, item);
    });
}
}