module;

#include <string_view>

export module cheat.hooks.c_base_animating.should_skip_animation_frame;
export import cheat.hooks.base;

export namespace cheat::hooks::c_base_animating
{
    struct should_skip_animation_frame : class_base
    {
        std::string_view name( ) const noexcept final;
    };
}