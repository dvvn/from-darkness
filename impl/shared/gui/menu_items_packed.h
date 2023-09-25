#pragma once

#include "menu_item_getter.h"
#include "container/array.h"
#include "functional/call_traits.h"

#include <algorithm>
#include <cassert>

namespace fd
{
template <size_t Count>
class menu_items_packed final : public menu_item_getter
{
    using pointer = function_info<menu_item_getter>::return_type;

    array<pointer, Count> items_;

  public:
    template <std::convertible_to<pointer>... T>
    menu_items_packed(T&&... items)
        : items_{items...}
    {
        if constexpr (Count == 1)
        {
            assert(items_[0] != nullptr);
        }
        else
        {
            namespace rn = std::ranges;
            assert(!rn::contains(items_, nullptr));
            assert(rn::all_of(items_, [&](auto* p) {
                return rn::count(items_, p) == 1;
            }));
        }
    }

    pointer operator()(size_t& counter) const override
    {
        return counter == Count ? nullptr : items_[counter++];
    }
};

template <>
class menu_items_packed<0>;

template <typename... T>
menu_items_packed(T&&...) -> menu_items_packed<sizeof...(T)>;
} // namespace fd