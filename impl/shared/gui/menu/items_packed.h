#pragma once

#include "item_getter.h"
#include "container/array.h"

#ifdef _DEBUG
#include <boost/hana/for_each.hpp>
#endif

#ifdef _DEBUG
#include <algorithm>
#endif
#include <cassert>

namespace fd
{
#ifdef _DEBUG
template <class... T>
class menu_items_packed
{
    using storage_type = menu_item_getter<T...>;

    storage_type items_;

  public:
    menu_items_packed(T... args)
        : items_(std::move(args)...)
    {
        constexpr auto pointers_count = (static_cast<size_t>(std::is_pointer_v<T>) + ...);
        if constexpr (pointers_count > 1)
        {
            void const* pointers[pointers_count];

            size_t offset = 0;
            boost::hana::for_each(items_, [&]<class C>(C& item) {
                if constexpr (std::is_pointer_v<C>)
                    pointers[offset++] = item;
            });

            assert(!std::ranges::contains(pointers, nullptr));
            std::ranges::sort(pointers);
            assert(std::ranges::adjacent_find(pointers) == std::end(pointers));
        }
    }

    storage_type& get()
    {
        return items_;
    }

    storage_type const& get() const
    {
        return items_;
    }
};

template <class... T, typename Invoker>
void apply(menu_items_packed<T...> const& items, Invoker invoker)
{
    return apply(items.get(), std::ref(invoker));
}

template <class... T>
void apply(menu_items_packed<T...> const& items)
{
    return apply(items.get());
}
#else
template <class... T>
using menu_items_packed = menu_item_getter<T...>;
#endif
} // namespace fd