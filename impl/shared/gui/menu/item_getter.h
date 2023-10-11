#pragma once

#include "functional/invoke.h"

#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

namespace fd
{
template <class... T>
using menu_item_getter = boost::hana::tuple<T...>;

inline constexpr auto render_menu_item = []<class T>(T& item) -> void {
    using raw_t = std::remove_pointer_t<T>;
    using std::invoke;
    using fd::invoke;
    if constexpr (std::invocable<raw_t>)
        invoke(item);
    else if constexpr (std::is_member_pointer_v<decltype(&raw_t::render)>)
        invoke(&raw_t::render, item);
    else
        invoke(&raw_t::render);
};

template <class... T>
void apply(menu_item_getter<T...> const& items)
{
    boost::hana::for_each(items, render_menu_item);
}

template <class... T>
void apply(menu_item_getter<T...>& items)
{
    boost::hana::for_each(items, render_menu_item);
}
}