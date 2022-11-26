#pragma once

#include <fd/hook_impl.h>

 namespace fd::hooked
{
    struct lock_cursor : hook_impl, hook_instance<lock_cursor>
    {
        lock_cursor(function_getter target);

        void callback() noexcept;
    };
} // namespace fd::hooks
