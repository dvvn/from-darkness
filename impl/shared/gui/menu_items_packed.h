#pragma once

#include "menu_item_getter.h"
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
template <size_t Count>
class menu_items_packed final : public menu_item_getter
{
    array<return_type, Count> items_;

  public:
    template <std::convertible_to<return_type>... T>
    menu_items_packed(T&&... items)
        : items_{items...}
    {
        if constexpr (Count == 1)
        {
            assert(items_[0] != nullptr);
        }
        else
        {
            assert(!std::ranges::contains(items_, nullptr));
            assert(std::ranges::all_of(items_, [&](auto* p) {
                return std::ranges::count(items_, p) == 1;
            }));
        }
    }

    return_type operator()(size_t& counter) const override
    {
        return counter == Count ? nullptr : items_[counter++];
    }
};

template <>
class menu_items_packed<0>;

template <typename... T>
menu_items_packed(T&&...) -> menu_items_packed<sizeof...(T)>;

#ifdef _DEBUG
template <class... T>
class menu_items_packed_new
{
    using storage_type = menu_item_getter_new<T...>;

    storage_type items_;

  public:
    menu_items_packed_new(T... args)
        : items_(std::move(args)...)
    {
        auto constexpr pointers_count = (static_cast<size_t>(std::is_pointer_v<T>) + ...);
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
void apply(menu_items_packed_new<T...> const& items, Invoker invoker)
{
    return apply(items.get(), std::ref(invoker));
}

template <class... T>
void apply(menu_items_packed_new<T...> const& items)
{
    return apply(items.get());
}
#else
template <class... T>
using menu_items_packed_new = menu_item_getter_new<T...>;
#endif
} // namespace fd