﻿#pragma once
#include "functional/invoke_on.h"
#include "noncopyable.h"

#include <atomic>

namespace fd
{
class basic_hook_callback : public noncopyable
{
    std::atomic_size_t called_;

  protected:
    ~basic_hook_callback();

  public:
    basic_hook_callback();

    void enter() noexcept;
    void exit() noexcept;
};

namespace detail
{
template <typename Callback>
concept callback_can_enter_exit = requires(Callback c) {
    c.enter();
    c.exit();
};

template <typename Callback, typename... Args>
decltype(auto) invoke_hook_callback(Callback& callback, Args&&... args)
{
    if constexpr (!callback_can_enter_exit<Callback&>)
    {
        return callback(std::forward<Args>(args)...);
    }
    else
    {
#ifdef _DEBUG
        using fn_ret = std::invoke_result_t<Callback&, Args&&...>;
        if constexpr (std::is_void_v<fn_ret>)
        {
            callback.enter();
            callback(std::forward<Args>(args)...);
            callback.exit();
        }
        else
#endif
        {
            callback.enter();
            invoke_on_destruct const lazy_exit{[cb = &callback] {
                cb->exit();
            }};
            return callback(std::forward<Args>(args)...);
        }
    }
}
} // namespace detail
} // namespace fd