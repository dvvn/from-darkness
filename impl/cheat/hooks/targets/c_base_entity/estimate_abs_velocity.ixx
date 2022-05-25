module;

#include <string_view>

export module cheat.hooks.c_base_entity.estimate_abs_velocity;
export import cheat.hooks.base;

export namespace cheat::hooks::c_base_entity
{
    struct estimate_abs_velocity : class_base
    {
        std::string_view name() const final;
    };
}
