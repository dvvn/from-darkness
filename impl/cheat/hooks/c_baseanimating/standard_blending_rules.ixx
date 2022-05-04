module;

#include <string_view>

export module cheat.hooks.c_base_animating.standard_blending_rules;
export import cheat.hooks.base;

export namespace cheat::hooks::c_base_animating
{
	class standard_blending_rules : public virtual class_base
	{
	public:
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}