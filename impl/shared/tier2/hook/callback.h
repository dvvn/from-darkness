#pragma once
#include "tier1/noncopyable.h"

#include <atomic>

namespace FD_TIER(2)
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

namespace detail
{
template <typename Callback>
concept callback_can_enter_exit = requires(Callback c) {
    c.enter();
    c.exit();
};
} // namespace detail
} // namespace FD_TIER(2)