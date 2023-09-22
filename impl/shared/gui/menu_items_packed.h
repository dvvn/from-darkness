#pragma once

#include "menu_item_getter.h"
#include "container/array.h"

namespace fd
{
template <size_t Count>
class menu_items_packed final : public menu_item_getter
{
    array<basic_menu_item*, Count> items_;

  public:
    menu_items_packed(auto*... items)
        : items_{items...}
    {
    }

    basic_menu_item* operator()(size_t& counter) const override
    {
        return counter == Count ? nullptr : items_[counter++];
    }
};

template <typename... T>
menu_items_packed(T...) -> menu_items_packed<sizeof...(T)>;

} // namespace fd