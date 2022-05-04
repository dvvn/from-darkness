module;

#include <string_view>

export module cheat.hooks.c_base_entity.estimate_abs_velocity;
export import cheat.hooks.base;

export namespace cheat::hooks::c_base_entity
{
	class estimate_abs_velocity : public virtual class_base
	{
	public:
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}