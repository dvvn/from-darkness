#pragma once

#include <fd/hook.h>
#include <fd/tuple.h>

#include <vector>

namespace fd
{
class hooks_storage final
{
    std::vector<basic_hook*> hooks_;

  public:
    hooks_storage() = default;

    void store(basic_hook* hook);
    void store(basic_hook& hook);

    bool enable();
    bool disable();
};

template <class H>
static bool _enable_hook(H& hook)
{
    return hook.active() || hook.enable();
}

template <class H>
static bool _disable_hook(H& hook)
{
    return !hook.active() || hook.disable();
}

template <class... H>
class hooks_storage2
{
    tuple<H...> hooks_;

  public:
    hooks_storage2(H... hooks)
        : hooks_(std::move(hooks)...)
    {
    }

    bool enable()
    {
        return apply(hooks_, [](H&... h) {
            return (h.enable() && ...);
        });
    }

    bool disable()
    {
        return apply(reversed(hooks_), [](auto&... h) {
            return (h.disable() & ...);
        });
    }
};
};