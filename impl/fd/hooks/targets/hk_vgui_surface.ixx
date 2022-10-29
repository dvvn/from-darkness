module;

export module fd.hooks.vgui_surface;
import fd.hooks.impl;

export namespace fd::hooks
{
    struct lock_cursor : impl
    {
        lock_cursor(function_getter target);
        lock_cursor(lock_cursor&& other);

      private:
        void callback();
    };
} // namespace fd::hooks
