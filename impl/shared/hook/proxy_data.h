#pragma once

#include "noncopyable.h"
#include "functional/cast.h"
#include "functional/ignore.h"

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
        original_ = unsafe_cast_lazy(original);
    }

    Fn get_original() const
    {
        return original_;
    }
};

template <typename Callback>
struct hook_proxy_data final : hook_proxy_callback<Callback>, hook_proxy_original<void*>, noncopyable
{
};

template <typename Callback>
inline hook_proxy_data<Callback> unique_hook_proxy_data;
}