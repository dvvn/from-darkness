module;

#include <string_view>

export module fds.hooks.studio_render.draw_model;
export import fds.hooks.base;

export namespace fds::hooks::studio_render
{
    struct draw_model : class_base
    {
        std::string_view name() const final;
    };
} // namespace fds::hooks::studio_render
