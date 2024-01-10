#pragma once

#include "hook/create.h"
#include "concepts.h"

namespace fd
{
namespace detail
{
template <typename T>
T* extract_pointer(T* ptr)
{
    return ptr;
}

template <contains_pointer T>
auto extract_pointer(T& obj) -> decltype(obj.get())
{
    return obj.get();
}

template <class T>
T* extract_pointer(T& obj)
{
    return &obj;
}
} // namespace detail

template <class Backend>
class create_hook_helper
{
    Backend backend_;

  public:
    create_hook_helper() = default;

    create_hook_helper(Backend backend)
        : backend_{std::move(backend)}
    {
    }

    auto operator->()
    {
        return detail::extract_pointer(backend_);
    }

    auto operator->() const
    {
        return detail::extract_pointer(backend_);
    }

    template <class Target, class Callback>
    bool operator()(Target const target, Callback* const callback)
    {
        auto info = prepare_hook<Callback>(target);
        return create_hook(operator->(), info, callback);
    }

    template <class Target, class Callback>
    bool operator()(Target const target, Callback* const callback) const requires requires { static_cast<void*>(operator->()); }
    {
        auto info = prepare_hook<Callback>(target);
        return create_hook(operator->(), info, callback);
    }
};

} // namespace fd