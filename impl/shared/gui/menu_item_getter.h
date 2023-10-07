#pragma once
#include "basic_menu_item.h"
#include "functional/basic_function.h"

#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

#include <concepts>
#include <functional>

namespace fd
{
using menu_item_getter = basic_function<basic_menu_item const*, size_t&>;

template <std::derived_from<menu_item_getter> T, typename Fn>
void apply(T const& items, Fn invoker)
{
    size_t counter = 0;
    for (;;)
    {
        auto item = std::invoke(items, counter);
        if (!item)
            break;
        std::invoke(invoker, item);
    }
}

template <std::derived_from<menu_item_getter> T>
void apply(T const& items)
{
    apply(items, &T::render);
}

template </*std::derived_from<basic_menu_item>*/ class... T>
using menu_item_getter_new = boost::hana::tuple<T...>;

template <class... T, typename Fn>
void apply(menu_item_getter_new<T...> const& items, Fn invoker)
{
    if constexpr ((std::invocable<Fn, T&> && ...) && 0)
        boost::hana::for_each(items, invoker);
    else
        boost::hana::for_each(items, [&invoker]<class C>(C& value) {
            if constexpr (std::is_pointer_v<C> && !std::invocable<Fn, C*>)
                invoker(*value);
            else
                invoker(value);
        });
}

template <class... T>
void apply(menu_item_getter_new<T...> const& items)
{
    boost::hana::for_each(items, []<class C>(C& item) {
        using raw_t = std::remove_pointer_t<C>;
        std::invoke(&raw_t::render, item);
    });
}
}