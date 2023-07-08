#pragma once

#include "interface.h"
#include "noncopyable.h"
#ifdef _DEBUG
#include "diagnostics/fatal.h"
#endif

#include "concepts.h"

#include <memory>

namespace fd
{
template <class T>
concept valid_unique_interface = (std::is_final_v<T> && std::derived_from<T, basic_stack_interface>) ||
                                 /* or has_virtual_destructor_v  */
                                 std::derived_from<T, basic_interface>;

enum class interface_type : uint8_t
{
    heap,
    in_place,
    stack,
};

template <interface_type Type, class T>
struct unique_interface;

namespace detail
{
template <class T>
struct delete_interface : std::default_delete<T>
{
    static_assert(valid_unique_interface<T>);

    using std::default_delete<T>::default_delete;
};

template <class T>
struct destroy_interface
{
    static_assert(valid_unique_interface<T>);

    constexpr destroy_interface() = default;

    template <std::derived_from<T> T2>
    constexpr destroy_interface(destroy_interface<T2>)
    {
    }

    constexpr void operator()(T *ptr) const
    {
        ptr->~T();
    }
};
} // namespace detail

template <class T>
struct unique_interface<interface_type::heap, T>
{
    using type = std::unique_ptr<T, detail::delete_interface<T>>;
};

template <class T>
struct unique_interface<interface_type::in_place, T>
{
    using type = std::unique_ptr<T, detail::destroy_interface<T>>;
};

constexpr interface_type default_interface_type = interface_type::heap;

template <interface_type Type, class T, bool = forwarded<T>>
struct construct_interface;

template <class T>
struct construct_interface<interface_type::heap, T, false>
{
    using element_type = T;
    using holder_type  = typename unique_interface<interface_type::heap, T>::type;

    template <typename... Args>
    static holder_type get(Args &&...args)
    {
        return holder_type(new element_type(std::forward<Args>(args)...));
    }
};

template <class T>
struct construct_interface<interface_type::in_place, T, false>
{
    using element_type = T;
    using holder_type  = typename unique_interface<interface_type::in_place, T>::type;

    template <typename... Args>
    static holder_type get(void *buffer, size_t buffer_size, Args &&...args)
    {
#ifdef _DEBUG
        if (buffer_size < sizeof(element_type))
            unreachable();
#else
        (void)buffer_size;
#endif
        return holder_type(new (buffer) T(std::forward<Args>(args)...));
    }
};

template <class T>
struct construct_interface<interface_type::stack, T, false>
{
    using element_type = T;

    template <typename... Args>
    static auto get(Args &&...args) -> element_type *
    {
        static element_type object(std::forward<Args>(args)...);
        return &object;
    }
};

#pragma region construct_interface_wrapper
#define FD_CONSTRUCT_INTERFACE(_T_, _IFC_)                                                   \
    template <>                                                                              \
    struct construct_interface<interface_type::heap, _T_>                                    \
    {                                                                                        \
        using element_type = _IFC_;                                                          \
        using holder_type  = unique_interface<interface_type::heap, element_type>::type;     \
        static holder_type get();                                                            \
    };                                                                                       \
    template <>                                                                              \
    struct construct_interface<interface_type::in_place, _T_>                                \
    {                                                                                        \
        using element_type = _IFC_;                                                          \
        using holder_type  = unique_interface<interface_type::in_place, element_type>::type; \
        static holder_type get(void *buffer, size_t buffer_length);                          \
    };                                                                                       \
    template <>                                                                              \
    struct construct_interface<interface_type::stack, _T_>                                   \
    {                                                                                        \
        using element_type = _IFC_;                                                          \
        static auto get() -> element_type *;                                                 \
    };

#define FD_CONSTRUCT_INTERFACE_IMPL(_T_)                                                                          \
    auto construct_interface<interface_type::heap, _T_>::get()->holder_type                                       \
    {                                                                                                             \
        return construct_interface<interface_type::heap, _T_, false>::get();                                      \
    }                                                                                                             \
    auto construct_interface<interface_type::in_place, _T_>::get(void *buffer, size_t buffer_length)->holder_type \
    {                                                                                                             \
        return construct_interface<interface_type::in_place, _T_, false>::get(buffer, buffer_length);             \
    }                                                                                                             \
    auto construct_interface<interface_type::stack, _T_>::get()->element_type *                                   \
    {                                                                                                             \
        return construct_interface<interface_type::stack, _T_, false>::get();                                     \
    }
#pragma endregion

template <interface_type Type, class T>
constexpr bool is_valid_interface_v = false;

template <class T>
constexpr bool is_valid_interface_v<interface_type::heap, T> = valid_unique_interface<T>;
template <class T>
constexpr bool is_valid_interface_v<interface_type::in_place, T> = valid_unique_interface<T>;
template <class T>
constexpr bool is_valid_interface_v<interface_type::stack, T> = std::derived_from<T, basic_stack_interface>;

namespace detail
{
inline void init_once()
{
#ifdef _DEBUG
    static auto used = false;
    if (used)
        unreachable();
    used = true;
#endif
}
} // namespace detail

template <class T, interface_type Type = default_interface_type, typename... Args>
auto make_interface(Args &&...args)
{
    using constructor = construct_interface<Type, T>;
    detail::init_once();
    static_assert(is_valid_interface_v<Type, typename constructor::element_type>);
    return constructor::get(std::forward<Args>(args)...);
}

} // namespace fd