#pragma once
#include "menu_items_packed.h"
#include "string/view.h"

namespace fd
{
template <size_t Count>
struct menu_item_data
{
    using items_packed = menu_items_packed<Count>;

  protected:
    string_view name;
    items_packed items;

  public:
    menu_item_data(string_view const name, items_packed const items)
        : name(name)
        , items(items)
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