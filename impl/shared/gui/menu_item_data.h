#pragma once
#include "menu_items_packed.h"
#include "string/view.h"

namespace fd
{
template <class Items>
struct menu_item_data
{
    string_view name;
    Items items;

    menu_item_data(string_view const name, Items items)
        : name(name)
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

// template <typename... T>
// menu_item_data(string_view, T*...)->menu_items_packed<sizeof...(T)>;
}