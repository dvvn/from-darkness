module;

#include <string_view>

export module cheat.hooks.client_mode.create_move;
export import cheat.hooks.base;

export namespace cheat::hooks::client_mode
{
	class create_move : public virtual class_base
	{
	public:
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}