#pragma once

#include <utility>

namespace fd
{
template <typename T>
constexpr T const* as_const(T* ptr)
{
    return ptr;
}

template <typename T>
constexpr T const* as_const(T const* ptr)
{
    return ptr;
}

using std::as_const;

template <typename T>
T* remove_const(T const* ptr)
{
    return const_cast<T*>(ptr);
}

template <typename T>
constexpr T* remove_const(T* ptr)
{
    return ptr;
}

template <typename T>
T& remove_const(T const& ref)
{
    return const_cast<T&>(ref);
}

template <typename T>
constexpr T& remove_const(T& ref)
{
    return (ref);
}

namespace detail
{
template <typename To, typename From>
concept can_static_cast = requires(From from) { static_cast<To>(from); };

inline void safe_cast_debug_trap()
{
}

template <typename To>
struct safe_cast_simple_ptr
{
    template <typename From>
    constexpr To operator()(From* from) const requires(can_static_cast<To, From*>)
    {
        safe_cast_debug_trap();
        return static_cast<To>(from);
    }
};

template <typename To>
struct safe_cast_simple_ptr<To const*>
{
    template <typename From>
    constexpr To const* operator()(From const* from) const requires(can_static_cast<To const*, From const*>)
    {
        safe_cast_debug_trap();
        return static_cast<To const*>(from);
    }
};

template <typename To>
struct safe_cast_simple_ref
{
    template <typename From>
    constexpr To operator()(From&& from) const requires(can_static_cast<To, From &&>)
    {
        safe_cast_debug_trap();
        return static_cast<To>(static_cast<From&&>(from));
    }
};

template <typename To>
struct safe_cast_simple
{
    template <typename From>
    constexpr To operator()(From from) const requires(can_static_cast<To, From>)
    {
        safe_cast_debug_trap();
        return static_cast<To>(from);
    }
};

template <typename P, size_t Count = 0>
struct pointers_count : std::integral_constant<size_t, Count>
{
};

template <typename P, size_t Count>
struct pointers_count<P*, Count> : pointers_count<P, Count + 1>
{
};

template <typename P, size_t Count>
struct pointers_count<P* const, Count> : pointers_count<P, Count + 1>
{
};

template <typename P, size_t Count = 1, bool = std::is_pointer_v<P>>
struct remove_pointer;

template <typename P>
struct remove_pointer<P, 0, true> : std::type_identity<P>
{
};

template <typename P, size_t Count>
struct remove_pointer<P, Count, false> : std::type_identity<P>
{
};

template <typename P, size_t Count>
struct remove_pointer<P* const, Count, true> : remove_pointer<P, Count - 1>
{
};

template <typename P, size_t Count>
struct remove_pointer<P*, Count, true> : remove_pointer<P, Count - 1>
{
};

template <typename P, size_t Count = 0>
using remove_pointer_t = typename remove_pointer<P, Count>::type;

template <typename P, size_t Index, size_t PointersCount = pointers_count<P>::value>
struct are_pointer_const : std::is_const<remove_pointer_t<P, PointersCount - Index>>
{
};

template <typename P, size_t Index, size_t PointersCount = pointers_count<P>::value>
inline constexpr bool are_pointer_const_v = are_pointer_const<P, Index, PointersCount>::value;

template <typename T, typename Ptr, size_t Index = 0, size_t Limit = pointers_count<Ptr>::value>
struct make_pointer_like : make_pointer_like<
                               std::conditional_t<are_pointer_const_v<Ptr, Index, Limit>, T const*, T*>, //
                               Ptr, Index + 1, Limit>
{
};

template <typename T, typename Ptr, size_t Limit>
struct make_pointer_like<T, Ptr, Limit, Limit> : std::type_identity<T>
{
};

template <typename T, typename P1, typename P2, size_t Index = 0, size_t Limit = pointers_count<P2>::value>
struct rewrap_pointer_like : rewrap_pointer_like<
                                 std::conditional_t<are_pointer_const_v<P1, Index, Limit> || are_pointer_const_v<P2, Index, Limit>, T const*, T*>, //
                                 P1, P2, Index + 1, Limit>
{
};

template <typename T, typename P1, typename P2, size_t Limit>
struct rewrap_pointer_like<T, P1, P2, Limit, Limit> : std::type_identity<T>
{
};

template <typename T, typename Ptr, size_t Limit>
struct make_pointer_like<T*, Ptr*, 0, Limit> : rewrap_pointer_like<remove_pointer_t<T, -1>, T*, Ptr*, 0, Limit>
{
};

template <typename T, typename Ptr, size_t Limit>
struct make_pointer_like<T* const, Ptr*, 0, Limit>; // dont use 'X* const'

template <typename T, typename Ptr, size_t Limit>
struct make_pointer_like<T**, Ptr*, 0, Limit>;
template <typename T, typename Ptr, size_t Limit>
struct make_pointer_like<T* const*, Ptr*, 0, Limit>;
template <typename T, typename Ptr, size_t Limit>
struct make_pointer_like<T*, Ptr**, 0, Limit>;
template <typename T, typename Ptr, size_t Limit>
struct make_pointer_like<T*, Ptr* const*, 0, Limit>;

template <typename To>
struct safe_cast_impl
{
    template <typename From, typename ToResolved = typename make_pointer_like<To, From*>::type>
    constexpr ToResolved operator()(From* from) const requires(can_static_cast<ToResolved, From*>)
    {
        safe_cast_debug_trap();
        return static_cast<ToResolved>(from);
    }
};

template <typename To>
struct safe_cast_impl<To*> : safe_cast_simple_ptr<To*>, safe_cast_simple_ptr<To const*>
{
};

template <typename To>
struct safe_cast_impl<To const*> : safe_cast_simple_ptr<To const*>
{
};

template <typename To>
struct safe_cast_impl<To&> : safe_cast_simple_ref<To&>
{
};

template <typename To>
struct safe_cast_impl<To const&> : safe_cast_simple_ref<To const&>
{
};

template <typename To>
struct safe_cast_impl<To&&> : safe_cast_simple_ref<To&&>
{
};
} // namespace detail

template <typename To>
inline constexpr detail::safe_cast_impl<To> safe_cast;

template <typename To, typename From>
To unsafe_cast(From from)
{
    static_assert(sizeof(From) == sizeof(To));

    union
    {
        From from0;
        To to;
    };

    from0 = from;
    return to;
}
} // namespace fd