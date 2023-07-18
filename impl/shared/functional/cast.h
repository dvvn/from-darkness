#pragma once

#include <utility>

namespace fd
{
template <typename T>
constexpr T const *as_const(T *ptr)
{
    return ptr;
}

template <typename T>
constexpr T const *as_const(T const *ptr)
{
    return ptr;
}

using std::as_const;

template <typename T>
T *remove_const(T const *ptr)
{
    return const_cast<T *>(ptr);
}

template <typename T>
constexpr T *remove_const(T *ptr)
{
    return ptr;
}

template <typename T>
T &remove_const(T const &ref)
{
    return const_cast<T &>(ref);
}

template <typename T>
constexpr T &remove_const(T &ref)
{
    return (ref);
}

template <typename To, typename From>
concept can_static_cast = requires(From from) { static_cast<To>(from); };

template <typename To, typename From>
concept can_const_cast = requires(From from) { const_cast<To>(from); };

template <typename To, typename From>
concept can_reinterpret_cast = requires(From from) { reinterpret_cast<To>(from); };

template <typename T>
inline constexpr bool is_void_pointer_v = false;

template <>
inline constexpr bool is_void_pointer_v<void *> = true;

template <>
inline constexpr bool is_void_pointer_v<void const *> = true;

template <typename T>
inline constexpr bool is_void_pointer_v<T **> = is_void_pointer_v<T *>;

namespace detail
{
template <typename T, size_t Num = 0>
struct pointers_count : std::integral_constant<size_t, Num>
{
};

template <typename T, size_t Num>
struct pointers_count<T *, Num> : pointers_count<T, Num + 1>
{
};

template <typename T, size_t Count = pointers_count<T>::value>
constexpr bool are_pointer_const(size_t const idx, size_t const offset = 0)
{
    using mutable_t = std::remove_const_t<T>;
    using raw_t     = std::remove_pointer_t<T>;

    auto const target_idx = Count - offset;
    return idx == target_idx ? !std::same_as<mutable_t, T> : are_pointer_const<raw_t, Count>(idx, offset + 1);
}

template <typename T, typename P>
struct add_pointer_like;

template <typename T, bool Const>
using add_const_pointer = //
    std::conditional_t<
        Const,
        std::add_pointer_t<std::add_const_t<T>>, //
        std::add_pointer_t<T>>;

template <typename T, typename P, size_t LastIdx = pointers_count<P>::value, size_t Idx = 0>
struct repack_pointer : repack_pointer<
                            add_const_pointer< //
                                T, are_pointer_const<P, LastIdx>(Idx)>,
                            P, LastIdx, Idx + 1>
{
};

template <typename T, typename P, size_t LastIdx>
struct repack_pointer<T, P, LastIdx, LastIdx>
{
    using type = T;
};

template <typename T, typename P>
using repack_pointer_t = typename repack_pointer<T, P>::type;

template <typename To, typename Sample>
struct change_type_as
{
    using type = To;
};

template <typename To, typename Sample>
struct change_type_as<To *, Sample const *>
{
    using type = add_const_pointer<To, true>;
};

template <typename To, typename Sample>
struct change_type_as<To &, Sample const &>
{
    using type = To const &;
};

template <typename To, typename Sample>
using change_type_as_t = typename change_type_as<To, Sample>::type;
} // namespace detail

template <
    typename To, typename From, //
    can_static_cast<From &&> Result = detail::change_type_as_t<To, From &&>>
constexpr Result safe_cast(From &&from)
{
    return static_cast<Result>(from);
}

template <
    typename To, typename From,
    can_static_cast<From *> Result = std::conditional_t<
        std::is_pointer_v<To>,                //
        detail::change_type_as_t<To, From *>, //
        detail::repack_pointer_t<To, From *>>>
constexpr Result safe_cast(From *from)
{
    return static_cast<Result>(from);
}

template <typename To, typename From>
To unsafe_cast(From from)
{
    static_assert(sizeof(To) == sizeof(From));

    if constexpr (can_static_cast<To, From>)
        return static_cast<To>(from);
    else if constexpr (can_const_cast<To, From>)
        return const_cast<To>(from);
    else if constexpr (can_reinterpret_cast<To, From>)
        return reinterpret_cast<To>(from);
    else
    {
        union
        {
            From from0;
            To to;
        };

        from0 = from;
        return to;
    }
}
} // namespace fd