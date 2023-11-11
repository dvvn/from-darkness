#pragma once
#include "noncopyable.h"
#include "functional/invoke_on.h"

#include <atomic>
#include <concepts>

namespace fd
{
class basic_hook_callback : public noncopyable
{
    std::memory_order order_;
    std::atomic<size_t> called_;

  protected:
    ~basic_hook_callback();

  public:
    basic_hook_callback();

    void enter() noexcept;
    void exit() noexcept;
};

template <std::derived_from<basic_hook_callback> Callback, typename... Args>
auto invoke_hook_callback(Callback& callback, Args&&... args)
{
    using fn_ret = std::invoke_result<Callback&, Args&&...>;
    if constexpr (std::is_void_v<fn_ret>)
    {
        callback.enter();
        callback(std::forward<Args>(args)...);
        callback.exit();
    }
    else
    {
        callback.enter();
        invoke_on const lazy_exit(object_state::destruct(), [&] {
            callback.exit();
        });
        return callback(std::forward<Args>(args)...);
    }
}
} // namespace fd