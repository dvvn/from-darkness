module;

#include <string_view>

export module cheat.hooks.client.frame_stage_notify;
export import cheat.hooks.base;

export namespace cheat::hooks::client
{
	class frame_stage_notify : public virtual class_base
	{
	public:
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}