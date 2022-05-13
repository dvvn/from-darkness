module;

#include <string_view>

export module cheat.hooks.directx.reset;
export import cheat.hooks.base;

export namespace cheat::hooks::directx
{
	struct reset : class_base
	{
		std::string_view name( ) const noexcept final;
	};
}