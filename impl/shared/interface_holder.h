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
class unique_interface : public noncopyable
{
    // simple 'unique_ptr' class

    T *object_;

  public:
    ~unique_interface()
    {
        using enum interface_type;
        if constexpr (Type == heap)
        {
            delete object_;
        }
        else if constexpr (Type == in_place)
        {
            if (object_)
                object_->~T();
        }
    }

    unique_interface(T *object)
        : object_(object)
    {
        static_assert(valid_unique_interface<T>);
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

constexpr interface_type default_interface_type = interface_type::heap;

template <interface_type Type, class T>
struct interface_creator;

template <class T>
class construct_interface
{
    static void init_once()
    {
#ifdef _DEBUG
        static auto used = false;
        if (used)
            unreachable();
        used = true;
#endif
    }

  public:
    template <typename Ret>
    static constexpr auto heap = []<typename... Args>(Args &&...args) -> Ret {
        init_once();
        return new T(static_cast<Args &&>(args)...);
    };
    template <typename Ret>
    static constexpr auto stack = []<typename... Args>(Args &&...args) -> Ret {
        init_once();
        static T object(static_cast<Args &&>(args)...);
        return &object;
    };
    template <typename Ret>
    static constexpr auto in_place = []<typename... Args>(void *buffer, size_t buffer_size, Args &&...args) -> Ret {
        init_once();
#ifdef _DEBUG
        if (buffer_size < sizeof(T))
            unreachable();
#else
        (void)buffer_size;
#endif
        return new (buffer) T(static_cast<Args &&>(args)...);
    };
};

template <class T, typename Ret = unique_interface<interface_type::heap, T>>
constexpr auto construct_interface_heap = construct_interface<T>::template heap<Ret>;
template <class T, typename Ret = T *>
constexpr auto construct_interface_stack = construct_interface<T>::template stack<Ret>;
template <class T, typename Ret = unique_interface<interface_type::in_place, T>>
constexpr auto construct_interface_in_place = construct_interface<T>::template in_place<Ret>;

template <class T>
struct interface_creator<interface_type::heap, T>
{
    static constexpr auto get = construct_interface_heap<T>;
};

template <class T>
struct interface_creator<interface_type::stack, T>
{
    static constexpr auto get = construct_interface_stack<T>;
};

template <class T>
struct interface_creator<interface_type::in_place, T>
{
    static constexpr auto get = construct_interface_in_place<T>;
};

template <interface_type Type, class T>
constexpr bool is_valid_interface_v = /*!forwarded<T> &&*/ [] {
    using enum interface_type;
    switch (Type)
    {
    case heap:
    case in_place:
        return valid_unique_interface<T>;
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
    return interface_creator<Type, T>::get(static_cast<Args &&>(args)...);
}

} // namespace fd