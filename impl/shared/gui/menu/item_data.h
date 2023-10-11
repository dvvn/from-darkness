#pragma once
#include "menu_items_packed.h"

namespace fd
{
template <class Name, class Items>
struct menu_item_data
{
    Name name;
    Items items;

    constexpr menu_item_data(Name name, Items items)
        : name(std::move(name))
        , items(std::move(items))
    {
    }

    /*template <typename... T>
    menu_item_data(string_view const name, T... items)
        : name(name)
        , items(items...)
    {
    }*/
};

template <class Name, class Items>
menu_item_data(Name&&, Items) -> menu_item_data<Name, Items>;
}