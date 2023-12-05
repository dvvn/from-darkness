#pragma once

#include "functional/ignore.h"
#include "noncopyable.h"

#include <memory>

namespace fd
{
namespace detail
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
} // namespace detail

template <typename Callback>
class hook_info
{
    void* target_;
    void* replace_;

#ifdef _DEBUG
    detail::hook_proxy_data<Callback>* proxy_data_;
#endif

  public:
    constexpr hook_info()
    {
        ignore_unused(this);
    }

    hook_info(void* target, void* replace)
        : target_(target)
        , replace_(replace)
#ifdef _DEBUG
        , proxy_data_(std::addressof(detail::unique_hook_proxy_data<Callback>))
#endif
    {
    }

    void* target() const
    {
        return target_;
    }

    void* replace() const
    {
        return replace_;
    }
};
} // namespace fd