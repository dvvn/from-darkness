module;

#include <string_view>

export module cheat.hooks.client.frame_stage_notify;
export import cheat.hooks.base;

export namespace cheat::hooks::client
{
	struct frame_stage_notify : class_base
	{
		std::string_view name( ) const noexcept final;
	};
}