#pragma once

#include <boost/core/noncopyable.hpp>

#include <utility>

namespace fd
{
using hook_name = char const *;

struct hook_id
{
    void *target;

#ifdef _DEBUG
    hook_name name;
#else
    static constexpr hook_name name = "Unnamed";
#endif

    hook_id(nullptr_t)
        : hook_id(nullptr, "Unnamed")
    {
    }

    hook_id(void *target, hook_name name)
        : target(target)
#ifdef _DEBUG
        , name(name)
#endif
    {
        (void)name;
    }

    explicit operator bool() const
    {
        return target != nullptr;
    }
};

constexpr size_t _internal_hook_words_begin = __LINE__ + 1;
hook_id create_hook(void *target, void *replace, hook_name name, void **trampoline);
bool enable_hook(hook_id id);
bool enable_hook_lazy(hook_id id);
bool disable_hook(hook_id id);
bool disable_hook_lazy(hook_id id);
bool apply_lazy_hooks();
bool enable_hooks();
bool disable_hooks();
constexpr size_t _internal_hook_words = __LINE__ - _internal_hook_words_begin;

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
} // namespace fd