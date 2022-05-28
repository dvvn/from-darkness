#pragma once

#include <cheat/core/object.h>

#include <nstd/runtime_assert.h>

#include <string_view>

import cheat.hooks.hook;

#define _FUNC_TYPE_member
#define _FUNC_TYPE_static static

#define _IS_STATIC_member false
#define _IS_STATIC_static true

// clang-format off
#define _NAMESPACE_STR(_L_, _R_) #_L_"::"#_R_
// clang-format on

#define CHEAT_HOOK_BEGIN(_HOOK_SOURCE_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)            \
    using namespace cheat;                                                                \
    using namespace hooks;                                                                \
    using namespace _HOOK_SOURCE_;                                                        \
    struct _HOOK_NAME_##_impl final : hook, hook_instance_##_FN_TYPE_<_HOOK_NAME_##_impl> \
    {                                                                                     \
        bool is_static() const override                                                   \
        {                                                                                 \
            return _IS_STATIC_##_FN_TYPE_;                                                \
        }                                                                                 \
        std::string_view name() const override                                            \
        {                                                                                 \
            return _NAMESPACE_STR(_HOOK_SOURCE_, _HOOK_NAME_);                            \
        }                                                                                 \
        void init() override;                                                             \
        _FUNC_TYPE_##_FN_TYPE_ _FN_RET_ callback(__VA_ARGS__);

#define CHEAT_HOOK_END(_HOOK_NAME_)                        \
    }                                                      \
    ; /*_HOOK_NAME_ -> one_instance_t::operator size_t()*/ \
    CHEAT_OBJECT_BIND(base, _HOOK_NAME_, _HOOK_NAME_##_impl)

#define CHEAT_HOOK_BODY(_HOOK_SOURCE_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)      \
    CHEAT_HOOK_BEGIN(_HOOK_SOURCE_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, __VA_ARGS__) \
    CHEAT_HOOK_END(_HOOK_NAME_)

#define CHEAT_HOOK_INIT(_HOOK_NAME_) void _HOOK_NAME_##_impl::init()
#define CHEAT_HOOK_CALLBACK(_HOOK_NAME_, _FN_RET_, ...) _FN_RET_ _HOOK_NAME_##_impl::callback(__VA_ARGS__)
