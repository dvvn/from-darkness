#pragma once

#include <utility>

namespace fd
{
template <typename... Args>
struct tuple;

/*template <size_t Idx, typename T, typename... Args>
static constexpr size_t _tuple_element_offset(const tuple<T, Args...>& tpl)
{
   if constexpr (Idx == 0)
       return 0;
   else
       return sizeof(T) + _tuple_element_offset<Idx - 1>(tpl.next);
}*/

template <size_t Idx, typename... Args>
struct tuple_index_type : tuple_index_type<Idx - 1, Args...>
{
};

template <typename T, typename... Args>
struct tuple_index_type<0, T, Args...>
{
    using type = T;
};

template <typename T, typename Type, typename... Args>
static constexpr size_t _tuple_type_index(const tuple<Type, Args...>& tpl)
{
    if constexpr (std::same_as<T, Type>)
        return 0;
    else
        return 1 + _tuple_type_index<T>(tpl.next);
}

template <size_t Idx, class Tpl>
static constexpr decltype(auto) _tuple_get_by_index(Tpl& tpl)
{
    if constexpr (Idx == 0)
        return tpl.get();
    else
        return _tuple_get_by_index<Idx - 1>(tpl.tail());
}

template <typename T, typename Tpl>
static constexpr decltype(auto) _tuple_get_by_type(Tpl& tpl)
{
    constexpr auto index = _tuple_type_index(std::declval<Tpl>());
    return _tuple_get_by_index<index>((tpl));
}

template <>
struct tuple<>
{
};

template <typename T, typename... Next>
struct tuple<T, Next...>
{
    using value_type = T;
    using tail_type  = tuple<Next...>;

  private:
    [[no_unique_address]] value_type value_;
    [[no_unique_address]] tail_type  tail_;

  public:
    template <typename V, typename... Args>
    constexpr tuple(V&& value, Args&&... next)
        : value_(std::forward<V>(value))
        , tail_(std::forward<Args>(next)...)
    {
    }

    constexpr value_type& get()
    {
        return value_;
    }

    constexpr const value_type& get() const
    {
        return value_;
    }

    constexpr tail_type& tail()
    {
        return tail_;
    }

    constexpr const tail_type& tail() const
    {
        return tail_;
    }
};

template <typename T, typename... Next>
struct tuple<T&, Next...>
{
    using value_type = T&;
    using tail_type  = tuple<Next...>;

  private:
    [[no_unique_address]] value_type value_;
    [[no_unique_address]] tail_type  tail_;

  public:
    template <typename V, typename... Args>
    constexpr tuple(V&& value, Args&&... next)
        : value_(std::forward<V>(value))
        , tail_(std::forward<Args>(next)...)
    {
    }

    constexpr value_type get() const
    {
        return value_;
    }

    constexpr tail_type& tail()
    {
        return tail_;
    }

    constexpr const tail_type& tail() const
    {
        return tail_;
    }
};

template <typename... Args>
tuple(Args&&...) -> tuple<std::decay_t<Args>...>;

template <typename... Args>
struct tuple_view : tuple<Args...>
{
    using tuple<Args...>::tuple;
};

template <typename... Args>
tuple_view(Args&&...) -> tuple_view<Args&&...>;

template <size_t Idx, typename... Args>
constexpr decltype(auto) get(tuple<Args...>& tpl)
{
    return _tuple_get_by_index<Idx>(tpl);
}

template <size_t Idx, typename... Args>
constexpr decltype(auto) get(const tuple<Args...>& tpl)
{
    return _tuple_get_by_index<Idx>(tpl);
}

template <typename T, typename... Args>
constexpr decltype(auto) get(tuple<Args...>& tpl)
{
    return _tuple_get_by_type<T>(tpl);
}

template <typename T, typename... Args>
constexpr decltype(auto) get(const tuple<Args...>& tpl)
{
    return _tuple_get_by_type<T>(tpl);
}

template <typename... Args>
constexpr size_t size(const tuple<Args...>&)
{
    return sizeof...(Args);
}

template <size_t Offset, typename Fn, class Tpl>
static constexpr void _tuple_iterate(Tpl& tpl, Fn& fn)
{
    fn(tpl.get());
    if constexpr (Offset > 0)
        _tuple_iterate<Offset - 1>(tpl.tail(), fn);
}

template <typename Fn, typename... Args>
constexpr void iterate(tuple<Args...>& tpl, Fn fn)
{
    _tuple_iterate<sizeof...(Args)>(tpl, fn);
}

template <typename Fn, typename... Args>
constexpr void iterate(const tuple<Args...>& tpl, Fn fn)
{
    _tuple_iterate<sizeof...(Args)>(tpl, fn);
}

template <class Tpl, typename Fn, size_t... I>
static constexpr decltype(auto) _tuple_apply(Tpl& tpl, Fn& fn, std::index_sequence<I...>)
{
    return fn(get<I>(tpl)...);
}

template <typename Fn, typename... Args>
constexpr decltype(auto) apply(tuple<Args...>& tpl, Fn fn)
{
    return _tuple_apply(tpl, fn, std::make_index_sequence<sizeof...(Args)>());
}

template <typename Fn, typename... Args>
constexpr decltype(auto) apply(const tuple<Args...>& tpl, Fn fn)
{
    return _tuple_apply(tpl, fn, std::make_index_sequence<sizeof...(Args)>());
}
}