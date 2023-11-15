#pragma once

#include "noncopyable.h"
#include "functional/cast.h"
#include "functional/ignore.h"

namespace fd::detail
{
template <typename Callback>
struct hook_proxy_callback
{
    Callback* callback;
};

template <typename Fn>
struct hook_proxy_original
{
    Fn original;
};

template <typename Callback>
struct hook_proxy_data final : hook_proxy_callback<Callback>, hook_proxy_original<void*>, noncopyable
{
};

template <typename Callback>
inline hook_proxy_data<Callback> unique_hook_proxy_data;
}