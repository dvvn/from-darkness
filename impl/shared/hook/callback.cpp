#include "callback.h"

#include <cassert>

namespace fd
{
basic_hook_callback::~basic_hook_callback()
{
    // in_use_.wait(true, order);
    do
    {
        // nothing here
    }
    while (called_.load(order) != 0);
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
}