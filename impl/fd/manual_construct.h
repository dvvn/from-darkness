#pragma once

#include <memory>

namespace fd
{
template <typename T>
concept has_destructor = requires(T obj) { obj.~T(); };

template <typename T>
union manual_construct
{
    uint8_t gap;
    T object;

    ~manual_construct()
    {
        if constexpr (has_destructor<T>)
            std::destroy_at(&object);
    }

    manual_construct()
    {
    }
};

template <typename T, typename... Args>
void construct_manual_object(T &obj, Args &&...args)
{
    if constexpr (sizeof...(Args) != 0)
    {
        std::destroy_at(&obj);
        std::construct_at(&obj, std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
void construct_manual_object(manual_construct<T> &obj, Args &&...args)
{
    std::construct_at(&obj.object, std::forward<Args>(args)...);
}

template <typename T>
T &extract_manual_object(T &val)
{
    return val;
}

template <typename T>
T &extract_manual_object(manual_construct<T> &obj)
{
    return obj.object;
}

template <typename T>
using manual_construct_t = std::conditional_t<std::default_initializable<T>, T, manual_construct<T>>;
}