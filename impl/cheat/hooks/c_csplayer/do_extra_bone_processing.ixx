module;

#include <string_view>

export module cheat.hooks.c_csplayer.do_extra_bone_processing;
export import cheat.hooks.base;

export namespace cheat::hooks::c_csplayer
{
	class do_extra_bone_processing : public virtual class_base
	{
	public:
		std::string_view class_name( ) const noexcept final;
		std::string_view function_name( ) const noexcept final;
	};
}