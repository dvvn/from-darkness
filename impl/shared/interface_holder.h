#pragma once

#include "concepts.h"
#include "interface.h"
#ifdef _DEBUG
#include "diagnostics/fatal.h"
#endif

#include <tuple>

namespace fd
{
enum class interface_type : uint8_t
{
    heap,
    in_place,
    stack,
};

template <interface_type Type, class T>
class unique_interface final
{
    T *interface_;

  public:
    ~unique_interface()
    {
        if constexpr (Type != interface_type::stack)
        {
            if (!interface_)
                return;
            interface_->~T();
        }
    }

    /*explicit*/ unique_interface(T *ifc)
        : interface_(ifc)
    {
        if constexpr (Type == interface_type::stack)
            static_assert(std::derived_from<T, basic_stack_interface>);
        else
            static_assert(std::derived_from<T, basic_interface>);
    }

    template <std::derived_from<T> T2>
    unique_interface(unique_interface<Type, T2> &&other)
        : unique_interface(other.release())
    {
    }

    unique_interface(unique_interface &&other) noexcept
        : interface_(other.release())
    {
    }

    unique_interface &operator=(unique_interface &&other) noexcept
    {
        using std::swap;
        swap(interface_, other.interface_);
        return *this;
    }

    T *release()
    {
        auto ret   = interface_;
        interface_ = nullptr;
        return ret;
    }

    operator T *() const
    {
        return interface_;
    }

    T &operator*() const
    {
        return *interface_;
    }

    T *operator->() const
    {
        return interface_;
    }
};

template <interface_type Type, class T, bool = forwarded<T>>
struct interface_creator;

#define CONSTRUCT_INTERFACE_PACKED                             \
    template <typename... Args>                                \
    requires(!std::constructible_from<T, std::tuple<Args...>>) \
    static holder_type get(std::tuple<Args...> &tpl)           \
    {                                                          \
        return std::apply(                                     \
            [](Args... args)                                   \
            {                                                  \
                /**/                                           \
                return get(std::forward<Args>(args)...);       \
            },                                                 \
            std::move(tpl));                                   \
    }                                                          \
    static holder_type get(std::tuple<>)                       \
    {                                                          \
        return get();                                          \
    }

template <interface_type Type, class T>
struct interface_holder
{
    using type = unique_interface<Type, T>;
};

template <class T>
struct interface_holder<interface_type::stack, T>
{
    using type = T *;
};

template <class T>
struct interface_creator<interface_type::heap, T, false>
{
    using holder_type = typename interface_holder<interface_type::heap, T>::type;

    template <typename... Args>
    static holder_type get(Args &&...args)
    {
        return new T(std::forward<Args>(args)...);
    }

    CONSTRUCT_INTERFACE_PACKED;
};

template <class T>
struct interface_creator<interface_type::in_place, T, false>
{
    using holder_type = typename interface_holder<interface_type::in_place, T>::type;

    template <typename... Args>
    static holder_type get(Args &&...args)
    {
        static uint8_t buffer[sizeof(T)];
        return new (&buffer) T(std::forward<Args>(args)...);
    }

    CONSTRUCT_INTERFACE_PACKED;
};

template <class T>
struct interface_creator<interface_type::stack, T, false>
{
    using holder_type = typename interface_holder<interface_type::stack, T>::type;

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

#define FD_INTERFACE_FWD0(_TYPE_, _T_, _IFC_, ...)                                          \
    template <>                                                                             \
    struct interface_creator<interface_type::_TYPE_, _T_>                                   \
    {                                                                                       \
        using holder_type = typename interface_holder<interface_type::_TYPE_, _IFC_>::type; \
        using args_packed = std::tuple<__VA_ARGS__> __VA_OPT__(&);                          \
        static holder_type get(args_packed args);                                           \
    };

#define FD_INTERFACE_FWD(_T_, _IFC_, ...) \
    FD_INTERFACE_FWD0(                    \
        heap, /**/                        \
        _T_, _IFC_, __VA_ARGS__);         \
    FD_INTERFACE_FWD0(                    \
        in_place, /**/                    \
        _T_, _IFC_, __VA_ARGS__);         \
    FD_INTERFACE_FWD0(                    \
        stack, /**/                       \
        _T_, _IFC_, __VA_ARGS__);

#define FD_INTERFACE_IMPL0(_TYPE_, _T_)                                                     \
    auto interface_creator<interface_type::_TYPE_, _T_>::get(args_packed args)->holder_type \
    {                                                                                       \
        return interface_creator<interface_type::_TYPE_, _T_, false>::get(args);            \
    }

#define FD_INTERFACE_IMPL(_T_)         \
    FD_INTERFACE_IMPL0(heap, _T_);     \
    FD_INTERFACE_IMPL0(in_place, _T_); \
    FD_INTERFACE_IMPL0(stack, _T_);
#pragma endregion

namespace detail
{
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

template <class Creator, typename... Args>
requires requires { typename Creator::args_packed; }
auto create_interface(Args &&...args) -> typename Creator::holder_type
{
    return Creator::get(Creator::args_packed(std::forward<Args>(args)...));
}

template <class Creator, typename... Args>
auto create_interface(Args &&...args) -> typename Creator::holder_type
{
    return Creator::get(std::forward<Args>(args)...);
}
} // namespace detail

template <class T, interface_type Type = interface_type::in_place, typename... Args>
auto make_interface(Args &&...args)
{
    detail::init_once<T>();
    return detail::create_interface<interface_creator<Type, T>>(std::forward<Args>(args)...);
}

} // namespace fd