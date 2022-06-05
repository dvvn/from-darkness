module;

#include <string_view>

export module fds.hooks.c_csplayer.do_extra_bone_processing;
export import fds.hooks.base;

export namespace fds::hooks::c_csplayer
{
    struct do_extra_bone_processing : class_base
    {
        std::string_view name() const final;
    };
} // namespace fds::hooks::c_csplayer
