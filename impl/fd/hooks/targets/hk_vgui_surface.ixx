module;

export module fd.hooks.vgui_surface;
import fd.hooks.impl;
import fd.valve.vgui_surface;

export namespace fd::hooks
{
    struct lock_cursor : impl
    {
        ~lock_cursor() override;

        lock_cursor(valve::vgui_surface* surface);
        lock_cursor(lock_cursor&& other);

        string_view name() const override;

      private:
        void callback();
    };
} // namespace fd::hooks
