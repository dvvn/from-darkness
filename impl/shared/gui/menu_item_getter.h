#pragma once
#include "basic_menu_item.h"
#include "functional/basic_function.h"

#include <concepts>
#include <functional>

namespace fd
{
using menu_item_getter = basic_function<basic_menu_item const*, size_t&>;

template <std::derived_from<menu_item_getter> T, typename Invoker>
void apply(T const& items, Invoker invoker)
{
    size_t counter = 0;
    for (;;)
    {
        using std::invoke;
        auto item = invoke(items, counter);
        if (!item)
            break;
        invoke(invoker, item);
    }
}

template <std::derived_from<menu_item_getter> T>
void apply(T const& items)
{
    apply(items, &basic_menu_item::render);
}

}