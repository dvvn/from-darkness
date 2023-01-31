#pragma once

#include <utility>

namespace fd
{
template <typename... Args>
struct tuple;

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
    template <typename... Args>
    constexpr tuple(value_type value, Args&&... next)
        : value_(static_cast<value_type>(value))
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

template <typename T, typename... Next>
struct tuple<T*, Next...>
{
    using value_type = T*;
    using tail_type  = tuple<Next...>;

  private:
    [[no_unique_address]] value_type value_;
    [[no_unique_address]] tail_type  tail_;

  public:
    template <typename... Args>
    constexpr tuple(value_type value, Args&&... next)
        : value_(static_cast<value_type>(value))
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
constexpr tuple<Args&&...> tuple_view(Args&&... args)
{
    return { std::forward<Args>(args)... };
}

//----

template <size_t Idx, typename... Args>
struct tuple_type_by_index : tuple_type_by_index<Idx - 1, Args...>
{
};

// template <size_t Idx, typename... Args>
// struct tuple_type_by_index<Idx, tuple<Args...>> : tuple_type_by_index<Idx, Args...>
// {
// };

template <typename T, typename... Args>
struct tuple_type_by_index<0, T, Args...>
{
    using type = T;
};

template <typename T, size_t Offset, typename... Args>
struct tuple_index_by_type;

// template <typename T, size_t Offset, typename Type, typename... Args>
// struct tuple_index_by_type<T, Offset, Type, Args...> : tuple_index_by_type<T, Offset + 1, Args...>
//{
// };
//
// template <size_t Offset, typename T, typename... Args>
// struct tuple_index_by_type<T, Offset, T, Args...>
//{
//     static constexpr size_t index = Offset;
// };

template <typename T, typename Type, typename... Args>
static constexpr size_t _tuple_find_index(const tuple<Type, Args...>& tpl)
{
    if constexpr (std::same_as<T, Type>)
        return 0;
    else
        return 1 + _tuple_find_index<T>(tpl.tail());
}

template <size_t Idx, class Tpl>
static constexpr decltype(auto) _tuple_get_by_index(Tpl& tpl)
{
    if constexpr (Idx == 0)
        return tpl.get();
    else
        return _tuple_get_by_index<Idx - 1>(tpl.tail());
}

template <typename T, typename Type, typename... Args>
static constexpr decltype(auto) _tuple_get_by_type(const tuple<Type, Args...>& tpl)
{
    if constexpr (std::same_as<T, Type>)
        return tpl.get();
    else
        return _tuple_get_by_type<T>(tpl.tail());
}

template <typename T, typename Type, typename... Args>
static constexpr decltype(auto) _tuple_get_by_type(tuple<Type, Args...>& tpl)
{
    decltype(auto) val = _tuple_get_by_type<T>(std::as_const(tpl));
    if constexpr (std::is_pointer_v<T> || std::is_reference_v<T>)
        return static_cast<T>(val);
    else
        return const_cast<T&>(val);
}

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

template <size_t Count, typename Fn, class Tpl>
static constexpr void _tuple_iterate(Tpl& tpl, Fn& fn)
{
    if constexpr (Count > 0)
    {
        fn(tpl.get());
        _tuple_iterate<Count - 1>(tpl.tail(), fn);
    }
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
    return fn(_tuple_get_by_index<I>(tpl)...);
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

template <typename Fn, typename... Args>
constexpr decltype(auto) apply(tuple<Args...>&& tpl, Fn fn)
{
    return _tuple_apply(tpl, fn, std::make_index_sequence<sizeof...(Args)>());
}

template <class Tpl, size_t... I>
static constexpr auto _reverse_tuple(Tpl& tpl, std::index_sequence<I...> seq)
{
    constexpr auto offset = seq.size() - 1;
    return tuple<decltype(_tuple_get_by_index<offset - I>(tpl))...>(_tuple_get_by_index<offset - I>(tpl)...);
}

template <class Tpl, size_t... I>
static constexpr auto _reverse_tuple_rvalue(Tpl& tpl, std::index_sequence<I...> seq)
{
    constexpr auto offset = seq.size() - 1;
    return tuple<decltype(_tuple_get_by_index<offset - I>(tpl))...>(std::move(_tuple_get_by_index<offset - I>(tpl))...);
}

template <typename... Args>
constexpr auto reverse(tuple<Args...>& tpl)
{
    return _reverse_tuple(tpl, std::make_index_sequence<sizeof...(Args)>());
}

template <typename... Args>
constexpr auto reverse(const tuple<Args...>& tpl)
{
    return _reverse_tuple(tpl, std::make_index_sequence<sizeof...(Args)>());
}

template <typename... Args>
constexpr auto reverse(tuple<Args...>&& tpl)
{
    return _reverse_tuple_rvalue(tpl, std::make_index_sequence<sizeof...(Args)>());
}

} // namespace fd

namespace std
{
template <typename... T>
struct tuple_size<fd::tuple<T...>>
{
    static constexpr size_t size = sizeof...(T);
};

template <size_t Idx, typename... T>
struct tuple_element<Idx, fd::tuple<T...>> : fd::tuple_type_by_index<Idx, T...>
{
};
}