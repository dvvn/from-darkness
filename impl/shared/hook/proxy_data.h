#pragma once

#include "noncopyable.h"
#include "functional/cast.h"
#include "functional/ignore.h"

#include <type_traits>

namespace fd::detail
{
template <typename Callback>
class hook_proxy_callback
{
    Callback* callback_;

  public:
    hook_proxy_callback()
    {
        ignore_unused(this);
    }

    void set_callback(Callback* callback)
    {
        callback_ = callback;
    }

    Callback& get_callback() const
    {
        return *callback_;
    }
};

template <typename Fn>
class hook_proxy_original
{
    Fn original_;

  public:
    hook_proxy_original()
    {
        ignore_unused(this);
    }

    void set_original(void* original)
    {
        original_ = unsafe_cast<Fn>(original);
    }

    Fn get_original() const
    {
        return original_;
    }
};

template <typename Callback>
struct hook_proxy_callback_for : std::type_identity<hook_proxy_callback<Callback>>
{
};

template <typename Callback>
struct hook_proxy_original_for : std::type_identity<hook_proxy_original<void*>>
{
};

template <typename Callback>
struct hook_proxy_data final : hook_proxy_callback_for<Callback>::type, hook_proxy_original_for<Callback>::type, noncopyable
{
};

template <typename Callback>
inline hook_proxy_data<Callback> unique_hook_proxy_data;

template <typename T>
inline uint8_t unique_hook_callback[sizeof(T)];
}