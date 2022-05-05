module;

#include <string_view>

export module cheat.hooks.winapi.wndproc;
export import cheat.hooks.base;

export namespace cheat::hooks::winapi
{
	struct wndproc : static_base
	{
		std::string_view function_name( ) const noexcept final;
	};
}