module;

#include <string_view>

export module cheat.hooks.vgui_surface.lock_cursor;
export import cheat.hooks.base;

export namespace cheat::hooks::vgui_surface
{
	struct lock_cursor : class_base
	{
        std::string_view name() const final;
    };
}
