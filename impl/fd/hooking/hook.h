#pragma once

#include <boost/core/noncopyable.hpp>
#include <boost/predef.h>

#include <utility>

#ifndef off
#define _OFF_DEFINED
#define off 1337
#endif

namespace fd
{
#if FD_DEFAULT_LOG_LEVEL == off
using hook_id = uintptr_t;
#else
#if BOOST_ARCH_X86 == BOOST_VERSION_NUMBER_AVAILABLE
using hook_id = uint64_t;
#elif BOOST_ARCH_X86_64 == BOOST_VERSION_NUMBER_AVAILABLE

#endif
#endif
using hook_name = char const *;

hook_id create_hook(void *target, void *replace, hook_name name, void **trampoline);
bool enable_hook(hook_id id);
bool enable_hook_lazy(hook_id id);
bool disable_hook(hook_id id);
bool disable_hook_lazy(hook_id id);
bool apply_lazy_hooks();
bool enable_hooks();
bool disable_hooks();

class hook : public boost::noncopyable
{
    hook_id id_;

  public:
    ~hook()
    {
        if (id_)
            disable_hook(id_);
    }

    hook(hook_id id)
        : id_(id)
    {
    }

    explicit operator bool() const
    {
        return id_;
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
        return disable_hook(std::exchange(id_, 0));
    }
};
} // namespace fd

#ifdef _OFF_DEFINED
#undef _OFF_DEFINED
#undef off
#endif