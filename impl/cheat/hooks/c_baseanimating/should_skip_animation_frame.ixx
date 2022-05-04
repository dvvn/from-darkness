module;

#include <string_view>

export module cheat.hooks.c_base_animating.should_skip_animation_frame;
export import cheat.hooks.base;

export namespace cheat::hooks::c_base_animating
{
	class should_skip_animation_frame : public virtual class_base
	{
	public:
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}