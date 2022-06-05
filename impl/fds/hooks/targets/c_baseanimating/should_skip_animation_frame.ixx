module;

#include <string_view>

export module fds.hooks.c_base_animating.should_skip_animation_frame;
export import fds.hooks.base;

export namespace fds::hooks::c_base_animating
{
    struct should_skip_animation_frame : class_base
    {
        std::string_view name() const final;
    };
} // namespace fds::hooks::c_base_animating
