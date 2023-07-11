#pragma once

#include "concepts.h"
#include "interface.h"
#include "noncopyable.h"
#ifdef _DEBUG
#include "diagnostics/fatal.h"
#endif

#include <memory>
#include <tuple>

namespace fd
{
enum class interface_type : uint8_t
{
    heap,
    in_place,
    stack,
};

inline constexpr interface_type default_interface_type = interface_type::in_place;

template <class T>
struct delete_interface final : std::default_delete<T>
{
    static_assert(std::derived_from<T, basic_interface>);

    using std::default_delete<T>::default_delete;
};

template <class T>
struct destroy_interface final
{
    static_assert(std::derived_from<T, basic_interface>);

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

template <class T>
T *operator&(std::unique_ptr<T, delete_interface<T>> &ptr)
{
    return ptr.get();
}

template <class T>
T *operator&(std::unique_ptr<T, destroy_interface<T>> &ptr)
{
    return ptr.get();
}

template <interface_type Type, class T, bool = forwarded<T>>
struct construct_interface;

#define CONSTRUCT_INTERFACE_PACKED                       \
    template <typename... Args>                          \
    static holder_type get(std::tuple<Args...> &tpl)     \
    {                                                    \
        return std::apply(                               \
            [](Args... args) {                           \
                /**/                                     \
                return get(std::forward<Args>(args)...); \
            },                                           \
            std::move(tpl)                               \
        );                                               \
    }                                                    \
    static holder_type get(std::tuple<>)                 \
    {                                                    \
        return get();                                    \
    }

template <class T>
struct construct_interface<interface_type::heap, T, false>
{
    using holder_type = std::unique_ptr<T, delete_interface<T>>;

    template <typename... Args>
    static holder_type get(Args &&...args)
    {
        return holder_type(new T(std::forward<Args>(args)...));
    }

    CONSTRUCT_INTERFACE_PACKED;
};

template <class T>
struct construct_interface<interface_type::in_place, T, false>
{
    using holder_type = std::unique_ptr<T, destroy_interface<T>>;

    template <typename... Args>
    static holder_type get(Args &&...args)
    {
        static uint8_t buffer[sizeof(T)];
        return holder_type(new (&buffer) T(std::forward<Args>(args)...));
    }

    CONSTRUCT_INTERFACE_PACKED;
};

template <class T>
struct construct_interface<interface_type::stack, T, false>
{
    using holder_type = T *;

    template <typename... Args>
    static holder_type get(Args &&...args)
    {
        static T object(std::forward<Args>(args)...);
        return &object;
    }

    CONSTRUCT_INTERFACE_PACKED;
};

#undef CONSTRUCT_INTERFACE_PACKED

#pragma region construct_interface_wrapper

#define FD_GROUP_ARGS(...) __VA_ARGS__

#define FD_INTERFACE_FWD0(_TYPE_, _T_, _IFC_, _HOLDER_, ...)       \
    template <>                                                    \
    struct construct_interface<interface_type::_TYPE_, _T_>        \
    {                                                              \
        using holder_type = _HOLDER_;                              \
        using args_packed = std::tuple<__VA_ARGS__> __VA_OPT__(&); \
        static holder_type get(args_packed args);                  \
    };

#define FD_INTERFACE_FWD(_T_, _IFC_, ...)                                \
    FD_INTERFACE_FWD0(                                                   \
        heap, /**/                                                       \
        _T_,                                                             \
        _IFC_,                                                           \
        FD_GROUP_ARGS(std::unique_ptr<_IFC_, delete_interface<_IFC_>>),  \
        __VA_ARGS__                                                      \
    );                                                                   \
    FD_INTERFACE_FWD0(                                                   \
        in_place, /**/                                                   \
        _T_,                                                             \
        _IFC_,                                                           \
        FD_GROUP_ARGS(std::unique_ptr<_IFC_, destroy_interface<_IFC_>>), \
        __VA_ARGS__                                                      \
    );                                                                   \
    FD_INTERFACE_FWD0(                                                   \
        stack, /**/                                                      \
        _T_,                                                             \
        _IFC_,                                                           \
        _IFC_ *,                                                         \
        __VA_ARGS__                                                      \
    );

#define FD_INTERFACE_IMPL0(_TYPE_, _T_)                                                       \
    auto construct_interface<interface_type::_TYPE_, _T_>::get(args_packed args)->holder_type \
    {                                                                                         \
        return construct_interface<interface_type::_TYPE_, _T_, false>::get(args);            \
    }

#define FD_INTERFACE_IMPL(_T_)         \
    FD_INTERFACE_IMPL0(heap, _T_);     \
    FD_INTERFACE_IMPL0(in_place, _T_); \
    FD_INTERFACE_IMPL0(stack, _T_);
#pragma endregion

namespace detail
{
template <interface_type Type, class T>
inline constexpr bool valid_interface_v = false;
template <interface_type Type, class T, class D>
inline constexpr bool valid_interface_v<Type, std::unique_ptr<T, D>> = valid_interface_v<Type, T>;
template <class T>
inline constexpr bool valid_interface_v<interface_type::heap, T> = std::derived_from<T, basic_interface>;
template <class T>
inline constexpr bool valid_interface_v<interface_type::in_place, T> = std::derived_from<T, basic_interface>;
template <class T>
inline constexpr bool valid_interface_v<interface_type::stack, T> = std::derived_from<T, basic_stack_interface>;

template <interface_type Type, class T>
concept valid_interface = valid_interface_v<Type, T>;

template <interface_type Type, class T>
concept valid_interface_holder = valid_interface_v<Type, typename construct_interface<Type, T>::holder_type>;

template <interface_type Type, class T>
concept pack_interface_args = requires { typename construct_interface<Type, T>::args_packed; };

template <typename>
void init_once()
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
    static_assert(detail::valid_interface_holder<Type, T>);
    detail::init_once<T>();
    using constructor = construct_interface<Type, T>;
    if constexpr (detail::pack_interface_args<Type, T>)
        return constructor::get(constructor::args_packed(std::forward<Args>(args)...));
    else
        return constructor::get(std::forward<Args>(args)...);
}

} // namespace fd