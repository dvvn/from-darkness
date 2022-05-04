module;

#include <string_view>

export module cheat.hooks.vgui_surface.lock_cursor;
export import cheat.hooks.base;

export namespace cheat::hooks::vgui_surface
{
	class lock_cursor : public virtual class_base
	{
	public:
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}