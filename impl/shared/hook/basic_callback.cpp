#include "basic_callback.h"

#include <cassert>

using std::memory_order_relaxed;

namespace fd
{
basic_hook_callback::~basic_hook_callback()
{
    // in_use_.wait(true, memory_order_relaxed);
    do
    {
        // nothing here
    }
    while (called_.load(memory_order_relaxed) != 0);
}

basic_hook_callback::basic_hook_callback()
    : called_(0)
{
    assert(called_.is_lock_free());
}

void basic_hook_callback::enter()
{
    ++called_;
}

void basic_hook_callback::exit()
{
    --called_;
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