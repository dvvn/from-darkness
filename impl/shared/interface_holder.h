#pragma once

#include "interface.h"
#include "noncopyable.h"
#ifdef _DEBUG
#include "diagnostics/fatal.h"
#endif

#include <concepts>

namespace fd
{
template <class T>
class unique_interface : public noncopyable
{
    // simple 'unique_ptr' class

    T *object_;

  public:
    ~unique_interface()
    {
        if (object_)
            delete object_;
    }

    unique_interface(T *object)
        : object_(object)
    {
        // or has_virtual_destructor_v
        static_assert(std::derived_from<T, basic_interface>);
    }

    constexpr unique_interface() = default;

    T *operator->() const
    {
        return object_;
    }

    T &operator*() const
    {
        return *object_;
    }
};

enum class interface_type : uint8_t
{
    heap,
    stack
};

constexpr interface_type default_interface_type = interface_type::heap;

template <interface_type Type, class T>
struct interface_creator;

template <class T>
struct interface_creator<interface_type::heap, T>
{
    using holder = unique_interface<T>;

    template <typename... Args>
    static holder get(Args &&...args)
    {
        return new T(static_cast<Args &&>(args)...);
    }
};

template <class T>
struct interface_creator<interface_type::stack, T>
{
    using pointer = T *;

    template <typename... Args>
    static pointer get(Args &&...args)
    {
        static T object(static_cast<Args &&>(args)...);
        return &object;
    }
};

namespace detail
{
template <typename>
void interface_init_once()
{
#ifdef _DEBUG
    static auto used = false;
    if (used)
        unreachable();
    used = true;
#endif
}
} // namespace detail

template <interface_type Type, class T>
constexpr bool is_valid_interface_v = /*!forwarded<T> &&*/ [] {
    using enum interface_type;
    switch (Type)
    {
    case heap:
        return std::derived_from<T, basic_interface>;
    case stack:
        return std::derived_from<T, basic_stack_interface>;
    default:
        return false;
    }
}();

template <class T, interface_type Type = default_interface_type, typename... Args>
auto make_interface(Args &&...args)
{
    static_assert(is_valid_interface_v<Type, T>);
    detail::interface_init_once<T>();
    return interface_creator<Type, T>::get(static_cast<Args &&>(args)...);
}

} // namespace fd