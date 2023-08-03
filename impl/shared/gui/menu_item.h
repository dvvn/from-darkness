#pragma once

#include "basic_menu_item.h"
#include "container/array.h"
#include "iterator/unwrap.h"
#include "string/view.h"

#include <boost/hana/functional/arg.hpp>
#include <boost/hana/functional/overload.hpp>
#include <boost/hana/tuple.hpp>

namespace fd
{
template <typename... T>
class joined_menu_items final : public basic_joined_menu_items
{
    static auto make_buffer(T &...args)
    {
        namespace hana = boost::hana;

        constexpr auto helper = hana::overload(
            [](basic_menu_item *) {
                return hana::false_c;
            },
            []<class C>(C &stored) -> C && {
                return static_cast<C &&>(stored);
            });
        return hana::make_tuple(helper(args)...);
    }

    using buffer_type = decltype(make_buffer(std::declval<T &>()...));
    using array_type  = array<basic_menu_item *, sizeof...(T)>;

    [[no_unique_address]] //
    buffer_type buffer_;
    array_type array_;

    static constexpr size_t buffer_length_ =
        (!std::convertible_to<decltype(std::declval<T>()), basic_menu_item *> + ...);

    array_type make_array(T &...args)
    {
        namespace hana = boost::hana;

        auto const extract = [&]<size_t Idx>(std::in_place_index_t<Idx>) -> basic_menu_item * {
            using stored_t = decltype(hana::at_c<Idx>(buffer_));
            if constexpr (std::same_as<hana::false_, stored_t>)
                return hana::arg<Idx + 1>(args...);
            else
                return &hana::at_c<Idx>(buffer_);
        };

        return [&]<size_t... I>(std::index_sequence<I...>) -> array_type {
            return {extract(std::in_place_index<I>)...};
        }(std::make_index_sequence<sizeof...(T)>());
    }

  public:
    joined_menu_items(T... args)
        : buffer_(make_buffer(args...))
        , array_(make_array(args...))
    {
    }

    iterator begin() const override
    {
        return iterator_to_raw_pointer(std::begin(array_));
    }

    iterator end() const override
    {
        
        return iterator_to_raw_pointer(std::end(array_));
    }
};

template <class Target = void, class T>
auto unwrap_menu_item(T &item)
{
    if constexpr (!std::is_pointer_v<T>)
    {
        return unwrap_menu_item(&item);
    }
    else if constexpr (std::is_void_v<Target>)
    {
        if constexpr (std::convertible_to<T, basic_menu_item *>)
            return static_cast<basic_menu_item *>(item);
        else if constexpr (std::convertible_to<T, basic_joined_menu_items *>)
            return static_cast<basic_joined_menu_items *>(item);
        else
            return nullptr;
    }
    else
    {
        if constexpr (std::convertible_to<T, Target *>)
            return static_cast<Target *>(item);
        else
            return nullptr;
    }
}

template <class T>
class menu_item final : public basic_menu_item
{
    string_view name_;
    T item_;

  public:
    menu_item(string_view name, T item)
        : name_(std::move(name))
        , item_(item)
    {
        static_assert(!std::is_null_pointer_v<decltype(unwrap_menu_item(item))>);
    }

    string_view name() const override
    {
        return name_;
    }

    void render() override
    {
    }

    basic_menu_item *child() const override
    {
        return unwrap_menu_item<basic_menu_item>(item_);
    }

    basic_joined_menu_items *child_joined() const override
    {
        return unwrap_menu_item<basic_joined_menu_items>(item_);
    }
};

// template <class T>
// menu_item(string_view,T *) -> menu_item<T>;

template <class T>
class unique_object;

template <class T>
menu_item(string_view, unique_object<T>) -> menu_item<T *>;

} // namespace fd
