module;

#include <string_view>

export module cheat.hooks.winapi.wndproc;
export import cheat.hooks.base;

export namespace cheat::hooks::winapi
{
	class wndproc : public virtual static_base
	{
	public:
		std::string_view function_name( ) const noexcept final;
	};
}