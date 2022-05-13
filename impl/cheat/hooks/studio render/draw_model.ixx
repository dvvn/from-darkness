module;

#include <string_view>

export module cheat.hooks.studio_render.draw_model;
export import cheat.hooks.base;

export namespace cheat::hooks::studio_render
{
	struct draw_model : class_base
	{
		std::string_view name( ) const noexcept final;
	};
}