#pragma once
#include "menu_items_packed.h"
#include "string/view.h"

namespace fd
{
template <class Items>
class menu_item_data
{
  protected:
    string_view name;
    Items items;

  public:
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