#include "basic_callback.h"

#include <cassert>

using std::memory_order_relaxed;

namespace fd
{
basic_hook_callback::~basic_hook_callback()
{
    // in_use_.wait(true, memory_order_relaxed);
    while (in_use_.load(memory_order_relaxed))
    {
        // nothing here
    }
}

basic_hook_callback::basic_hook_callback()
    : in_use_(false)
{
    assert(in_use_.is_lock_free());
}

void basic_hook_callback::enter()
{
    in_use_.exchange(true, memory_order_relaxed);
}

void basic_hook_callback::exit()
{
    in_use_.exchange(false, memory_order_relaxed);
    // in_use_.notify_one();
}

hook_callback_call::~hook_callback_call()
{
    if (callback_)
        callback_->exit();
}

hook_callback_call::hook_callback_call(basic_hook_callback *callback)
    : callback_(callback)
{
    callback_->enter();
}

}