module;

#include <string_view>

export module cheat.hooks.c_base_animating.standard_blending_rules;
export import cheat.hooks.base;

export namespace cheat::hooks::c_base_animating
{
	struct standard_blending_rules : class_base
	{
        std::string_view name() const final;
    };
}
