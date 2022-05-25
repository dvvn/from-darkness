module;

#include <string_view>

export module cheat.hooks.c_csplayer.do_extra_bone_processing;
export import cheat.hooks.base;

export namespace cheat::hooks::c_csplayer
{
	struct do_extra_bone_processing : class_base
	{
        std::string_view name() const final;
    };
}
