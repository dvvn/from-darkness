#pragma once

#include "source.h"

#include <boost/container/static_vector.hpp>

#include <memory>
#include <string>

namespace fd
{
template <class T>
concept netvar_type_have_include = requires() { T::include; };

template <class S>
// for arrays, to store array and internal include
using netvar_type_merged_includes = boost::container::static_vector<S, 2>;

struct native_netvar_type
{
    std::string_view type;

    explicit constexpr native_netvar_type(std::string_view type)
        : type(type)
    {
    }
};

struct platform_netvar_type : native_netvar_type
{
    static constexpr std::string_view include = "<cstdint>";

    using native_netvar_type::native_netvar_type;
};

template <class T, class I>
struct custom_netvar_type
{
    T type;
    I include;

    template <typename Tt = T, typename Ii = I>
    explicit constexpr custom_netvar_type(Tt &&t, Ii &&incl)
        : type(std::forward<Tt>(t))
        , include(std::forward<Ii>(incl))
    {
    }
};

template <class T, class I>
struct custom_netvar_type<T, netvar_type_merged_includes<I>>
{
    T type;
    netvar_type_merged_includes<I> include;

    template <typename Tt>
    explicit custom_netvar_type(Tt &&t)
        : type(std::forward<Tt>(t))
    {
    }

    explicit operator bool() const
    {
        return !include.empty();
    }
};

template <typename T>
using string_or_view = std::conditional_t<std::is_trivially_destructible_v<T>, std::string_view, std::string>;

template <class T, class I>
custom_netvar_type(T type, I include) -> custom_netvar_type<string_or_view<T>, string_or_view<I>>;

using custom_netvar_type_simple = custom_netvar_type<std::string_view, std::string_view>;

// using known_netvar_type = std::variant<native_netvar_type, platform_netvar_type, custom_netvar_type_simple>;

struct netvar_type;

struct netvar_type_array
{
    std::string type;
    static constexpr std::string_view include = "<array>";

    using data_type = std::unique_ptr<netvar_type>;

    uint16_t size;
    data_type inner;

    explicit netvar_type_array(uint16_t size, netvar_type &&type);

    void fill(bool force = false);
};

template <class T>
struct netvar_includes_writer
{
    T it;

    void operator()(std::monostate) const
    {
        std::unreachable();
    }

    void operator()(native_netvar_type const &) const
    {
    }

    void operator()(platform_netvar_type const &val)
    {
        *it++ = val.include;
    }

    template <typename Type>
    void operator()(custom_netvar_type<Type, std::string> const &val)
    {
        *it++ = val.include;
    }

    template <typename Type>
    void operator()(custom_netvar_type<Type, std::string_view> const &val)
    {
        *it++ = val.include;
    }

    template <class Type, class Include>
    void operator()(custom_netvar_type<Type, netvar_type_merged_includes<Include>> const &val)
    {
        for (auto &inc : val.include)
            *it++ = inc;
    }

    void operator()(netvar_type_array const &arr);
};

template <class T>
netvar_includes_writer(T &&) -> netvar_includes_writer<std::decay_t<T>>;

struct netvar_type final
{
    using data_type = std::variant<
        std::monostate,
        native_netvar_type,
        platform_netvar_type,
        custom_netvar_type<std::string_view, std::string_view>,
        custom_netvar_type<std::string, std::string>,
        custom_netvar_type<std::string_view, std::string>,
        custom_netvar_type<std::string, std::string_view>,
        custom_netvar_type<std::string_view, netvar_type_merged_includes<std::string_view>>,
        custom_netvar_type<std::string, netvar_type_merged_includes<std::string>>,
        custom_netvar_type<std::string_view, netvar_type_merged_includes<std::string>>,
        custom_netvar_type<std::string, netvar_type_merged_includes<std::string_view>>,
        netvar_type_array>;

    data_type data;

    netvar_type() = default;

    template <typename T>
    netvar_type(T &&obj) requires(std::constructible_from<data_type, T &&>)
        : data(std::forward<T>(obj))
    {
    }

    template <typename T, std::ranges::range I>
    netvar_type(custom_netvar_type<T, I> obj) requires(!std::constructible_from<data_type, custom_netvar_type<T, I> &&>)
    {
        using str_t = std::ranges::range_value_t<I>;
        using inc_t = netvar_type_merged_includes<str_t>;
        inc_t inc;
        for (auto &v : obj.include)
            inc.emplace_back(std::move(v));
        using val_t = custom_netvar_type<T, inc_t>;
        data.emplace<val_t>(std::move(obj.type), std::move(inc));
    }

    std::string_view get_type();
    std::string_view get_type() const;

    template <class T>
    void write_includes_to(T &&it) const
    {
        std::visit(netvar_includes_writer<T &&>(std::forward<T>(it)), data);
    }
};

template <class T>
void netvar_includes_writer<T>::operator()(netvar_type_array const &arr)
{
    *it++ = arr.include;
    std::visit(*this, arr.inner->data);
}
}