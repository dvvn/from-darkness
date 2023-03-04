#pragma once

#include <fd/netvars/source.h>

#include <boost/container/static_vector.hpp>

#include <memory>
#include <string>
#include <vector>

namespace fd
{
template <class T>
concept netvar_type_have_include = requires() { T::include; };

struct native_netvar_type
{
    std::string_view type;
};

struct platform_netvar_type : native_netvar_type
{
};

template <class T, class I>
struct custom_netvar_type
{
    T type;
    I include;
};

template <typename T>
using string_or_view = std::conditional_t<std::is_trivially_destructible_v<T>, std::string_view, std::string>;

template <class T, class I>
custom_netvar_type(T type, I include) -> custom_netvar_type<string_or_view<T>, string_or_view<I>>;

using custom_netvar_type_simple = custom_netvar_type<std::string_view, std::string_view>;

using known_netvar_type = std::variant<native_netvar_type, platform_netvar_type, custom_netvar_type_simple>;

template <class S>
// for arrays, to store array and internal include
using netvar_type_merged_includes = boost::container::static_vector<S, 2>;

struct netvar_type;

struct netvar_type_array
{
    std::string type;

    using data_type = std::unique_ptr<netvar_type>;

    uint16_t  size;
    data_type data;

    netvar_type_array(uint16_t size, netvar_type&& type);

    void fill(bool force = false);
};

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
    netvar_type(T&& obj) requires(std::constructible_from<data_type, T &&>)
        : data(std::forward<T>(obj))
    {
    }

    std::string_view get_type() const;
};

template <class T>
concept know_netvar_include = requires(T const& val, std::vector<std::string_view> vec) {
    get_netvar_include(val, vec);
};

template <typename T, typename V>
concept can_emplace_back = requires(T obj, V val) { obj.emplace_back(static_cast<V>(val)); };

template <typename T, typename V>
void emplace_or_assign(T& buff, V&& val)
{
    if constexpr (can_emplace_back<T, V&&>)
        buff.emplace_back(static_cast<V&&>(val));
    else
        buff = static_cast<V&&>(val);
}

void get_netvar_include(platform_netvar_type const&, auto& buff)
{
    emplace_or_assign(buff, "<cstdint>");
}

void get_netvar_include(custom_netvar_type_simple const& type, auto& buff)
{
    emplace_or_assign(buff, type.include);
}

void get_netvar_include(known_netvar_type const& type, auto& buff)
{
    std::visit(
        [&]<class T>(T& t)
        {
            if constexpr (know_netvar_include<T>)
                get_netvar_include(t, buff);
        },
        type);
}

template <class T, class I>
void get_netvar_include(custom_netvar_type<T, I> const& type, auto& buff)
{
    if constexpr (can_emplace_back<I, char const*>)
        for (auto& str : type.includes)
            buff.emplace_back(str);
    else
        emplace_or_assign(buff, type.include);
}

void get_netvar_include(netvar_type_array const& type, auto& buff)
{
    buff.emplace_back("<array>");
    for (auto q = &type;; q = &std::get<netvar_type_array>(q->data->data))
    {
        if (!std::holds_alternative<netvar_type_array>(q->data->data))
        {
            get_netvar_include(*q->data, buff);
            break;
        }
    }
}

void get_netvar_include(netvar_type const& type, auto& buff)
{
    std::visit(
        [&]<class T>(T const& t)
        {
            if constexpr (know_netvar_include<T>)
                get_netvar_include(t, buff);
        },
        type.data);
}
}