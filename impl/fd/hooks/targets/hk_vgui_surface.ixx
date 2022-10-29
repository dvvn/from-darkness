module;

export module fd.hooks.vgui_surface;
import fd.hooks.impl;

export namespace fd::hooks
{
    struct lock_cursor : impl, instance<lock_cursor>
    {
        lock_cursor(function_getter target);

        void callback() noexcept;
    };
} // namespace fd::hooks
