#pragma once

#include "hook/proxy_data.h"

#include <memory>

namespace fd
{
template <typename Callback>
class hook_info
{
    void* target_;
    void* replace_;

#ifdef _DEBUG
    detail::hook_proxy_data<Callback>* proxy_data_;
#endif

  public:
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