#pragma once

#include <fds/core/object.h>
#include <fds/core/object_ex.h>

#include <array>
#include <limits>
//#include <source_location>

import fds.hook;

#define FN_member
#define FN_static static

#define IS_STATIC_member false
#define IS_STATIC_static true

template <size_t BuffSize>
constexpr auto _Hook_name_buff(const std::string_view name, const std::string_view group)
{
    std::array<char, BuffSize + 2 + 1> buff{};

    const auto buff_group_bg     = buff.begin();
    const auto buff_namespace_bg = buff_group_bg + group.size();
    const auto buff_name_bg      = buff_namespace_bg + 2;

    std::copy(group.begin(), group.end(), buff_group_bg);
    std::fill_n(buff_namespace_bg, 2, ':');
    std::copy(name.begin(), name.end(), buff_name_bg);
    buff.back() = '\0';
    return buff;
}

#define _Hook_name(_HOOK_NAME_)                                          \
    [] {                                                                 \
        constexpr std::string_view name  = FDS_AUTO_OBJECT_NAME;         \
        constexpr std::string_view group = FDS_AUTO_OBJECT_LOCATION;     \
        return _Hook_name_buff<name.size() + group.size()>(name, group); \
    }()

/* template <size_t S>
constexpr auto _Hook_name(const char (&name)[S])
{
    constexpr std::string_view group = _Folder_name(std::source_location::current().file_name());
    return _Hook_name_buff<S - 1 + group.size()>(name, group);
} */

#define FN_member
#define FN_static static

#define IS_STATIC_member false
#define IS_STATIC_static true

#define FDS_HOOK(_TARGET_FN_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)                                     \
    struct _HOOK_NAME_##_impl final : fds::hook_impl, fds::hook_instance_##_FN_TYPE_<_HOOK_NAME_##_impl> \
    {                                                                                                    \
        bool is_static() const override                                                                  \
        {                                                                                                \
            return IS_STATIC_##_FN_TYPE_;                                                                \
        }                                                                                                \
        std::string_view name() const override                                                           \
        {                                                                                                \
            static constexpr auto debug_name = _Hook_name(#_HOOK_NAME_);                                 \
            return {debug_name.data(), debug_name.size() - 1};                                           \
        }                                                                                                \
        void init() override                                                                             \
        {                                                                                                \
            hook::init(_TARGET_FN_, &_HOOK_NAME_##_impl::callback);                                      \
        }                                                                                                \
        FN_##_FN_TYPE_ _FN_RET_ callback(__VA_ARGS__);                                                   \
    };                                                                                                   \
    FDS_OBJECT_BIND(fds::hook_base, FDS_AUTO_OBJECT_ID, _HOOK_NAME_##_impl);                             \
    _FN_RET_ _HOOK_NAME_##_impl::callback(__VA_ARGS__)
