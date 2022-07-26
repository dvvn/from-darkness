#pragma once

#include <fd/object.h>
#include <fd/object_ex.h>

#include <limits>
#include <ranges>
//#include <source_location>

import fd.hook;
import fd.chars_cache;
import fd.format;

#define FN_member
#define FN_static static

#define IS_STATIC_member false
#define IS_STATIC_static true

namespace fd
{
    constexpr auto _Underline_to_space(const char chr)
    {
        return chr == '_' ? ' ' : chr;
    }

#define _Hook_name                                                                                                                                                \
    [] {                                                                                                                                                          \
        constexpr auto group          = FD_AUTO_OBJECT_LOCATION;                                                                                                  \
        constexpr auto name           = FD_AUTO_OBJECT_NAME;                                                                                                      \
        constexpr auto hook_name_size = group.size() + 2 + name.size();                                                                                           \
        const auto hook_name          = make_string(group | std::views::transform(_Underline_to_space), "::", name | std::views::transform(_Underline_to_space)); \
        return chars_cache<char, hook_name_size + 1>(hook_name.data(), hook_name_size);                                                                           \
    }

    /* template <size_t S>
    constexpr auto _Hook_name(const char (&name)[S])
    {
        constexpr string_view group = _Folder_name(std::source_location::current().file_name());
        return _Hook_name_buff<S - 1 + group.size()>(name, group);
    } */

} // namespace fd

#define FN_member
#define FN_static static

#define IS_STATIC_member false
#define IS_STATIC_static true

#define FD_HOOK_HEAD(_FN_TYPE_)                                                                                    \
    constexpr auto hook_index = /* FD_UNIQUE_INDEX */ FD_AUTO_OBJECT_ID;                                           \
    template <>                                                                                                    \
    struct hook_impl_unique<hook_index> final : hook_impl, hook_instance_##_FN_TYPE_<hook_impl_unique<hook_index>> \
    {                                                                                                              \
        bool is_static() const override                                                                            \
        {                                                                                                          \
            return IS_STATIC_##_FN_TYPE_;                                                                          \
        }                                                                                                          \
        string_view name() const override                                                                          \
        {                                                                                                          \
            return chars_cache_buff<_Hook_name()>;                                                                 \
        }

#define FD_HOOK_INIT()                                                               \
    void init() override                                                             \
    {                                                                                \
        hook_impl::init(this->target_fn(), &hook_impl_unique<hook_index>::callback); \
    }                                                                                \
    function_getter target_fn()

#define FD_HOOK_CALLBACK(_FN_TYPE_, _FN_RET_, ...) \
    /**/                                           \
    FN_##_FN_TYPE_ _FN_RET_ callback(__VA_ARGS__)

#define FD_HOOK_TAIL() \
    }                  \
    ;                  \
    FD_OBJECT_IMPL(hook_base, FD_OBJECT_GET(hook_impl_unique<hook_index>), hook_index);

//---

#define FD_HOOK_SIG(_MODULE_NAME_, _SIG_, _FN_TYPE_, _FN_RET_, ...) \
    FD_HOOK_HEAD(_FN_TYPE_)                                         \
    FD_HOOK_INIT()                                                  \
    {                                                               \
        return rt_modules::_MODULE_NAME_.find_signature<_SIG_>();   \
    }                                                               \
    FD_HOOK_CALLBACK(_FN_TYPE_, _FN_RET_, __VA_ARGS__);             \
    FD_HOOK_TAIL();                                                 \
    _FN_RET_ hook_impl_unique<hook_index>::callback(__VA_ARGS__)

#define FD_HOOK_VTABLE(_IFC_NAME_, _INDEX_, _FN_RET_, ...) \
    FD_HOOK_HEAD(member)                                   \
    FD_HOOK_INIT()                                         \
    {                                                      \
        return { &FD_OBJECT_GET(_IFC_NAME_*), _INDEX_ };   \
    }                                                      \
    FD_HOOK_CALLBACK(member, _FN_RET_, __VA_ARGS__);       \
    FD_HOOK_TAIL();                                        \
    _FN_RET_ hook_impl_unique<hook_index>::callback(__VA_ARGS__)

#define FD_HOOK(_TARGET_FN_, _FN_TYPE_, _FN_RET_, ...)                         \
    FD_HOOK_HEAD(_FN_TYPE_)                                                    \
    void init() override                                                       \
    {                                                                          \
        hook_impl::init(_TARGET_FN_, &hook_impl_unique<hook_index>::callback); \
    }                                                                          \
    FD_HOOK_CALLBACK(_FN_TYPE_, _FN_RET_, __VA_ARGS__);                        \
    FD_HOOK_TAIL();                                                            \
    _FN_RET_ hook_impl_unique<hook_index>::callback(__VA_ARGS__)
