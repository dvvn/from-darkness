module;

#include <string_view>

export module fds.hooks.c_base_entity.estimate_abs_velocity;
export import fds.hooks.base;

export namespace fds::hooks::c_base_entity
{
    struct estimate_abs_velocity : class_base
    {
        std::string_view name() const final;
    };
} // namespace fds::hooks::c_base_entity
