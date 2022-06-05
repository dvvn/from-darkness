module;

#include <string_view>

export module fds.hooks.vgui_surface.lock_cursor;
export import fds.hooks.base;

export namespace fds::hooks::vgui_surface
{
    struct lock_cursor : class_base
    {
        std::string_view name() const final;
    };
} // namespace fds::hooks::vgui_surface
