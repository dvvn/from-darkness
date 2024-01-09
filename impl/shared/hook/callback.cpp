#include "hook/callback.h"

namespace fd
{
basic_hook_callback::~basic_hook_callback()
{
    // in_use_.wait(true, order);
    do
    {
        // nothing here
    }
    while (called_.load(std::memory_order_relaxed) != 0);
}

basic_hook_callback::basic_hook_callback()
    : called_{0}
{
}

void basic_hook_callback::enter() noexcept
{
    ++called_;
}

void basic_hook_callback::exit() noexcept
{
    --called_;
    // in_use_.notify_one();
}
} // namespace fd