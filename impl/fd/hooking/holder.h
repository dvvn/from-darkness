#pragma once

#include "hook.h"

#include <boost/core/noncopyable.hpp>

#include <utility>

namespace fd
{
class hook_holder : public boost::noncopyable
{
    hook_id id_;

  public:
    ~hook()
    {
        if (id_)
            disable_hook(id_);
    }

    hook_holder(hook_id id)
        : id_(id)
    {
    }

    explicit operator bool() const
    {
        return static_cast<bool>(id_);
    }

    bool enable() const
    {
        return enable_hook(id_);
    }

    bool enable_lazy() const
    {
        return enable_hook_lazy(id_);
    }

    bool disable_lazy() const
    {
        return disable_hook_lazy(id_);
    }

    bool disable() const
    {
        return disable_hook(id_);
    }

    bool destroy()
    {
        return disable_hook(std::exchange(id_, nullptr));
    }
};
}