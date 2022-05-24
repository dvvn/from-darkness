#pragma once

#include <cheat/core/object.h>

#include <string_view>

import cheat.tools.object_name;
import cheat.hooks.hook;

#define CHEAT_HOOK(_FN_, _TYPE_) struct _FN_##_impl final : _FN_, hook, hook_instance_##_TYPE_<_FN_##_impl>

#define CHEAT_HOOK_IMPL(_FN_)                    \
    std::string_view _FN_::name() const noexcept \
    {                                            \
        return tools::csgo_object_name<_FN_>;    \
    }                                            \
    CHEAT_OBJECT_IMPL(_FN_, _FN_##_impl::get_ptr(), 0)
