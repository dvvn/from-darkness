#pragma once
#include "functional/invoke_cast.h"

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

template <typename T>
constexpr T const& as_const(T& ptr)
{
    return ptr;
}

template <typename T>
constexpr T const& as_const(T const& ptr)
{
    return ptr;
}

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
concept can_static_cast = !std::is_void_v<To> && requires(From from) { static_cast<To>(from); };

constexpr void safe_cast_debug_trap()
{
}

inline void unsafe_cast_debug_trap()
{
}

template <typename To>
struct safe_cast_simple
{
    template <typename From>
    constexpr To operator()(From from) const
#ifdef _DEBUG
        requires(can_static_cast<To, From>)
#endif
    {
        safe_cast_debug_trap();
        return static_cast<To>(from);
    }
};

template <typename To>
struct safe_cast_simple_ptr
{
    template <typename From>
    constexpr To operator()(From* from) const
#ifdef _DEBUG
        requires(can_static_cast<To, From*>)
#endif
    {
        safe_cast_debug_trap();
        return static_cast<To>(from);
    }
};

template <typename To>
struct safe_cast_simple_ptr<To const*>
{
    template <typename From>
    constexpr To const* operator()(From const* from) const
#ifdef _DEBUG
        requires(can_static_cast<To const*, From const*>)
#endif
    {
        safe_cast_debug_trap();
        return static_cast<To const*>(from);
    }
};

template <typename To>
struct safe_cast_simple_ref
{
    template <typename From>
    constexpr To operator()(From&& from) const
#ifdef _DEBUG
        requires(can_static_cast<To, From &&>)
#endif
    {
        safe_cast_debug_trap();
        return static_cast<To>(static_cast<From&&>(from));
    }
};

template <typename P, size_t Count = 0>
struct pointers_count : integral_constant<size_t, Count>
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
struct remove_pointer<P, 0, true> : type_identity<P>
{
};

template <typename P, size_t Count>
struct remove_pointer<P, Count, false> : type_identity<P>
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
struct make_pointer_like :
    make_pointer_like<
        conditional_t<are_pointer_const_v<Ptr, Index, Limit>, T const*, T*>, //
        Ptr, Index + 1, Limit>
{
};

template <typename T, typename Ptr, size_t Limit>
struct make_pointer_like<T, Ptr, Limit, Limit> : type_identity<T>
{
};

template <typename T, typename P1, typename P2, size_t Index = 0, size_t Limit = pointers_count<P2>::value>
struct rewrap_pointer_like :
    rewrap_pointer_like<
        conditional_t<are_pointer_const_v<P1, Index, Limit> || are_pointer_const_v<P2, Index, Limit>, T const*, T*>, //
        P1, P2, Index + 1, Limit>
{
};

template <typename T, typename P1, typename P2, size_t Limit>
struct rewrap_pointer_like<T, P1, P2, Limit, Limit> : type_identity<T>
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
struct safe_cast_ptr_rewrap
{
    template <typename From, typename ToResolved = typename make_pointer_like<To, From*>::type>
    constexpr ToResolved operator()(From* from) const
#ifdef _DEBUG
        requires(can_static_cast<ToResolved, From*>)
#endif
    {
        safe_cast_debug_trap();
        return static_cast<ToResolved>(from);
    }
};

template <typename To>
struct safe_cast_impl : safe_cast_ptr_rewrap<To>, safe_cast_simple<To>
{
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

template <typename To>
struct unsafe_cast_direct
{
    template <typename From>
    To operator()(From from) const requires(sizeof(To) == sizeof(From) && can_static_cast<To, From>)
    {
        unsafe_cast_debug_trap();
        return static_cast<To>(from);
    }
};

template <typename To>
struct unsafe_cast_force
{
    template <typename From>
    To operator()(From from) const requires(sizeof(To) == sizeof(From) && !can_static_cast<To, From> && std::is_trivially_destructible_v<From>)
    {
        unsafe_cast_debug_trap();

        union
        {
            uint8_t gap = 0;
            From from0;
            To to;
        };

        from0 = from;
        return to;
    }
};

template <typename To, bool Trivial = std::is_trivially_destructible_v<To>>
struct unsafe_cast_impl;

template <typename To>
struct unsafe_cast_impl<To, true> : unsafe_cast_direct<To>, unsafe_cast_force<To>
{
};

template <typename To>
struct unsafe_cast_impl<To, false> : unsafe_cast_direct<To>
{
};

template <typename From, template <typename> class Impl>
struct custom_cast
{
    From from;

    template <typename To>
    To operator()(type_identity<To>) const
#ifdef _DEBUG
        requires(std::invocable<Impl<To>, From const&>)
#endif
    {
        constexpr Impl<To> impl;
        return impl(from);
    }

    template <typename To>
    To operator()(type_identity<To>)
#ifdef _DEBUG
        requires(std::invocable<Impl<To>, From&>)
#endif
    {
        constexpr Impl<To> impl;
        return impl(from);
    }
};

template <template <typename> class Impl>
inline constexpr auto cast_to = []<typename From>(From from) -> invoke_cast<custom_cast<From, Impl>> {
    return {from};
};
} // namespace detail

template <typename T>
struct remove_rvalue_reference : type_identity<T>
{
};

template <typename T>
struct remove_rvalue_reference<T&> : type_identity<T&>
{
};

template <typename T>
struct remove_rvalue_reference<T&&> : type_identity<T>
{
};

template <typename To>
inline constexpr detail::safe_cast_impl<To> safe_cast;
inline constexpr auto safe_cast_from = detail::cast_to<detail::safe_cast_impl>;
template <typename To>
inline constexpr detail::unsafe_cast_impl<To> unsafe_cast;
inline constexpr auto unsafe_cast_from = detail::cast_to<detail::unsafe_cast_impl>;

template <typename To, typename From>
using safe_cast_result = std::invoke_result_t<detail::safe_cast_impl<To>, From>;
} // namespace fd
