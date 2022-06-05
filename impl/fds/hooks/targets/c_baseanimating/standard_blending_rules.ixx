module;

#include <string_view>

export module fds.hooks.c_base_animating.standard_blending_rules;
export import fds.hooks.base;

export namespace fds::hooks::c_base_animating
{
    struct standard_blending_rules : class_base
    {
        std::string_view name() const final;
    };
} // namespace fds::hooks::c_base_animating
