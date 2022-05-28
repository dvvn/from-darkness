#pragma once

#include <cheat/core/object.h>

#include <nstd/runtime_assert.h>

#include <string_view>

import cheat.tools.object_name;
import cheat.hooks.hook;

#define FN_TYPE_member
#define FN_TYPE_static static

#define CHEAT_HOOK_BEGIN(_HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)                              \
    struct _HOOK_NAME_##_impl final : hook, hook_instance_##_FN_TYPE_<_HOOK_NAME_##_impl> \
    {                                                                                        \
        bool is_static() const override                                                      \
        {                                                                                    \
            return /* #_FN_TYPE_ == "static" */ #_FN_TYPE_[0] == 's';                        \
        }                                                                                    \
        std::string_view name() const override                                               \
        {                                                                                    \
            constexpr auto objname = tools::object_name<_HOOK_NAME_##_impl>;                 \
            return {objname.begin(), objname.end() - 5}; /*remove '_impl'*/                  \
        }                                                                                    \
        void init() override;                                                                \
        FN_TYPE_##_FN_TYPE_ _FN_RET_ callback(__VA_ARGS__);

#define CHEAT_HOOK_END(_HOOK_NAME_)                      \
    }                                                    \
    ;                                                    \
    /*_HOOK_NAME_ -> one_instance_t::operator size_t()*/ \
    CHEAT_OBJECT_BIND(base, _HOOK_NAME_, _HOOK_NAME_##_impl)

#define CHEAT_HOOK_BODY(_HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)      \
    CHEAT_HOOK_BEGIN(_HOOK_NAME_, _FN_TYPE_, _FN_RET_, __VA_ARGS__) \
    CHEAT_HOOK_END(_HOOK_NAME_)

#define CHEAT_HOOK_INIT(_HOOK_NAME_) void _HOOK_NAME_##_impl::init()
#define CHEAT_HOOK_CALLBACK(_HOOK_NAME_, _FN_RET_, ...) _FN_RET_ _HOOK_NAME_##_impl::callback(__VA_ARGS__)
