module;

#include <string_view>

export module cheat.hooks.directx.present;
export import cheat.hooks.base;

export namespace cheat::hooks::directx
{
	struct present : class_base
	{
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}