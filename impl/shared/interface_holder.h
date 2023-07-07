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
concept valid_unique_interface = //
    std::is_final_v<T> ? std::derived_from<T, basic_stack_interface>
                       : std::derived_from<T, basic_interface> /* or has_virtual_destructor_v  */;

template <class T>
class unique_heap_interface : public noncopyable
{
    // simple 'unique_ptr' class

    T *object_;

  public:
    ~unique_heap_interface()
    {
        if (object_)
            delete object_;
    }

    unique_heap_interface(T *object)
        : object_(object)
    {
        static_assert(valid_unique_interface<T>);
    }

    constexpr unique_heap_interface() = default;

    T *operator->() const
    {
        return object_;
    }

    T &operator*() const
    {
        return *object_;
    }
};

template <class T>
class unique_stack_interface : public noncopyable
{
    // simple 'unique_ptr' class

    T *object_;

  public:
    ~unique_stack_interface()
    {
        if (object_)
            object_->~T();
    }

    unique_stack_interface(T *object)
        : object_(object)
    {
        static_assert(valid_unique_interface<T>);
    }

    constexpr unique_stack_interface() = default;

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
    in_place,
    stack,
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
    static constexpr auto heap = []<typename... Args>(Args &&...args) -> T * {
        init_once();
        return new T(static_cast<Args &&>(args)...);
    };

    static constexpr auto stack = []<typename... Args>(Args &&...args) -> T * {
        static T object(static_cast<Args &&>(args)...);
        init_once();
        return &object;
    };

    static constexpr auto in_place = []<typename... Args>(void *buffer, size_t buffer_size, Args &&...args) -> T * {
    // static_assert(std::derived_from<T, basic_stack_interface>);
#ifdef _DEBUG
        if (buffer_size < sizeof(T))
            unreachable();
#else
        (void)buffer_size;
#endif
        init_once();
        return new (buffer) T(static_cast<Args &&>(args)...);
    };
};

template <class T>
struct interface_creator<interface_type::heap, T>
{
    static constexpr auto get = construct_interface<T>::heap;
};

template <class T>
struct interface_creator<interface_type::stack, T>
{
    static constexpr auto get = construct_interface<T>::stack;
};

template <class T>
struct interface_creator<interface_type::in_place, T>
{
    static constexpr auto get = construct_interface<T>::in_place;
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