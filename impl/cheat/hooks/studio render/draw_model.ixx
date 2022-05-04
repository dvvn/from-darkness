module;

#include <string_view>

export module cheat.hooks.studio_render.draw_model;
export import cheat.hooks.base;

export namespace cheat::hooks::studio_render
{
	class draw_model : public virtual class_base
	{
	public:
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}