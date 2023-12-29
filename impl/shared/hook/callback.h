#pragma once
#include "functional/invoke_on.h"
#include "noncopyable.h"

#include <atomic>

namespace fd
{
class basic_hook_callback : public noncopyable
{
    static constexpr auto memory_order = std::memory_order_relaxed;

    std::atomic_size_t called_;

  protected:
    ~basic_hook_callback()
    {
        // in_use_.wait(true, order);
        do
        {
            // nothing here
        }
        while (called_.load(memory_order) != 0);
    }

  public:
    basic_hook_callback()
        : called_{0}
    {
    }

    void enter() noexcept
    {
        ++called_;
    }

    void exit() noexcept
    {
        --called_;
        // in_use_.notify_one();
    }
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
    if constexpr (!detail::callback_can_enter_exit<Callback&>)
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
            invoke_on const lazy_exit{
                object_state::destruct{}, //
                [cb = &callback] {
                    cb->exit();
                }};
            return callback(std::forward<Args>(args)...);
        }
    }
}
} // namespace detail
} // namespace fd