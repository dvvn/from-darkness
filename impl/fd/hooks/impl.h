#pragma once

#include <fd/object.h>
#include <fd/object_ex.h>

#include <ranges>
//#include <source_location>

import fd.hook;
import fd.chars_cache;
import fd.format;

#define FN_member
#define FN_static static

#define SPLIT_member "::"
#define SPLIT_static "."

#define IS_STATIC_member false
#define IS_STATIC_static true

#define FD_HOOKS_SINGLE_FILE

namespace fd
{
    constexpr auto _Underline_to_space(const char chr)
    {
        return chr == '_' ? ' ' : chr;
    }

#if 0

    template <size_t BuffSize>
    consteval auto _Hook_name(const fd::string_view group, const fd::string_view name)
    {
        chars_cache<BuffSize + 2 + 1> buff;
        buff.append(group | transform(_Underline_to_space)).append("::", 2).append(name | transform(_Underline_to_space));
        return buff;
    }

    template <size_t S, size_t S1>
    consteval auto _Hook_name(const char (&group)[S], const char (&name)[S1])
    {
        return _Hook_name<S - 1 + S1 - 1>({ group, S - 1 }, { name, S1 - 1 });
    }

#else

#define _NON_EMPTY(_CUSTOM_, _AUTO_) _CUSTOM_[0] == '\0' ? _AUTO_ : _CUSTOM_

#ifdef FD_HOOKS_SINGLE_FILE
#define _HOOK_AUTO_GROUP_NAME FD_AUTO_OBJECT_NAME
#define _HOOK_AUTO_NAME        \
    []() -> string_view {      \
        return { nullptr, 1 }; \
    }()
#else
#define _HOOK_AUTO_GROUP_NAME FD_AUTO_OBJECT_LOCATION
#define _HOOK_AUTO_NAME       FD_AUTO_OBJECT_NAME
#endif

#define _Hook_name(_GROUP_, _SPLIT_, _NAME_, ...)                                                                        \
    [] {                                                                                                                 \
        constexpr auto group        = _NON_EMPTY(#_GROUP_, _HOOK_AUTO_GROUP_NAME);                                       \
        constexpr string_view split = _SPLIT_;                                                                           \
        constexpr auto name         = _NON_EMPTY(#_NAME_, _HOOK_AUTO_NAME);                                              \
        constexpr auto buff_size    = group.size() + split.size() + name.size();                                         \
        chars_cache<char, buff_size + 1> buff;                                                                           \
        using std::views::transform;                                                                                     \
        buff.append(group | transform(_Underline_to_space)).append(split).append(name | transform(_Underline_to_space)); \
        return buff;                                                                                                     \
    }()

#endif
} // namespace fd

#define FN_member
#define FN_static static

#define SPLIT_member "::"
#define SPLIT_static "."

#define IS_STATIC_member false
#define IS_STATIC_static true

#define _HOOK_NAME(_N_) hk_##_N_

#define FD_HOOK_NAME(...)                                 \
    string_view name() const override                     \
    {                                                     \
        return chars_cache_buff<_Hook_name(__VA_ARGS__)>; \
    }

#define FD_HOOK_HEAD(_NAME_, _FN_TYPE_)                                                        \
    struct _HOOK_NAME(_NAME_) final : hook_impl, hook_instance_##_FN_TYPE_<_HOOK_NAME(_NAME_)> \
    {                                                                                          \
        using _This = _HOOK_NAME(_NAME_);                                                      \
        bool is_static() const override                                                        \
        {                                                                                      \
            return IS_STATIC_##_FN_TYPE_;                                                      \
        }

#define FD_HOOK_INIT()                                        \
    void init() override                                      \
    {                                                         \
        hook_impl::init(this->target_fn(), &_This::callback); \
    }                                                         \
    function_getter target_fn()

#define FD_HOOK_CALLBACK(_FN_TYPE_, _FN_RET_, ...) \
    /**/                                           \
    FN_##_FN_TYPE_ _FN_RET_ callback(__VA_ARGS__)

#define FD_HOOK_TAIL(_NAME_) \
    }                        \
    ;                        \
    /* FD_OBJECT_IMPL(hook_base, FD_OBJECT_GET(_NAME_), _INDEX_); */ FD_OBJECT_ATTACH(_HOOK_NAME(_NAME_)*, _HOOK_NAME(_NAME_));

//---

#define FD_HOOK_SIG(_NAME_, _MODULE_NAME_, _SIG_, _FN_TYPE_, _FN_RET_, ...) \
    FD_HOOK_HEAD(_NAME_, _FN_TYPE_)                                         \
    FD_HOOK_NAME(, SPLIT_##_FN_TYPE_, _NAME_)                               \
    FD_HOOK_INIT()                                                          \
    {                                                                       \
        return rt_modules::_MODULE_NAME_.find_signature<_SIG_>();           \
    }                                                                       \
    FD_HOOK_CALLBACK(_FN_TYPE_, _FN_RET_, __VA_ARGS__);                     \
    FD_HOOK_TAIL(_NAME_);                                                   \
    _FN_RET_ _HOOK_NAME(_NAME_)::callback(__VA_ARGS__)

#define FD_HOOK_VTABLE(_IFC_NAME_, _FN_NAME_, _INDEX_, _FN_RET_, ...) \
    FD_HOOK_HEAD(_IFC_NAME_##_##_FN_NAME_, member)                    \
    FD_HOOK_NAME(_IFC_NAME_, SPLIT_member, _FN_NAME_)                 \
    FD_HOOK_INIT()                                                    \
    {                                                                 \
        return { &FD_OBJECT_GET(_IFC_NAME_*), _INDEX_ };              \
    }                                                                 \
    FD_HOOK_CALLBACK(member, _FN_RET_, __VA_ARGS__);                  \
    FD_HOOK_TAIL(_IFC_NAME_##_##_FN_NAME_);                           \
    _FN_RET_ _HOOK_NAME(_IFC_NAME_##_##_FN_NAME_)::callback(__VA_ARGS__)

#define FD_HOOK(_NAME_, _TARGET_FN_, _FN_TYPE_, _FN_RET_, ...) \
    FD_HOOK_HEAD(_NAME_, _FN_TYPE_)                            \
    FD_HOOK_NAME(, SPLIT_##_FN_TYPE_, _NAME_)                  \
    FD_HOOK_INIT()                                             \
    {                                                          \
        return _TARGET_FN_;                                    \
    }                                                          \
    FD_HOOK_CALLBACK(_FN_TYPE_, _FN_RET_, __VA_ARGS__);        \
    FD_HOOK_TAIL(_NAME_);                                      \
    _FN_RET_ _HOOK_NAME(_NAME_)::callback(__VA_ARGS__)
