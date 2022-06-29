#pragma once

#include <fd/object.h>
#include <fd/object_ex.h>

#include <array>
#include <limits>
//#include <source_location>

import fd.hook;

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

    constexpr auto write = []<typename It, typename It2>(It begin, const It end, It2 dest) {
        do
        {
            const auto chr = *begin++;
            *dest++        = chr == '_' ? ' ' : chr;
        }
        while (begin != end);
    };

    /* std::copy */ write(group.begin(), group.end(), buff_group_bg);
    std::fill_n(buff_namespace_bg, 2, ':');
    /* std::copy */ write(name.begin(), name.end(), buff_name_bg);
    buff.back() = '\0';
    return buff;
}

#define _Hook_name                                                       \
    [] {                                                                 \
        constexpr std::string_view name  = FD_AUTO_OBJECT_NAME;          \
        constexpr std::string_view group = FD_AUTO_OBJECT_LOCATION;      \
        return _Hook_name_buff<name.size() + group.size()>(name, group); \
    }

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

#define _FD_HOOK(_TARGET_FN_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)                                  \
    constexpr auto _hook_index = /* FD_UNIQUE_INDEX */ FD_AUTO_OBJECT_ID;                             \
    template <size_t Index>                                                                           \
    struct _HOOK_NAME_ final : fd::hook_impl, fd::hook_instance_##_FN_TYPE_<_HOOK_NAME_<_hook_index>> \
    {                                                                                                 \
        bool is_static() const override                                                               \
        {                                                                                             \
            return IS_STATIC_##_FN_TYPE_;                                                             \
        }                                                                                             \
        std::string_view name() const override                                                        \
        {                                                                                             \
            static constexpr auto hook_name = _Hook_name();                                           \
            return { hook_name.data(), hook_name.size() - 1 };                                        \
        }                                                                                             \
        void init() override                                                                          \
        {                                                                                             \
            hook_impl::init(_TARGET_FN_, &_HOOK_NAME_::callback);                                     \
        }                                                                                             \
        FN_##_FN_TYPE_ _FN_RET_ callback(__VA_ARGS__);                                                \
    };                                                                                                \
    FD_OBJECT_IMPL(fd::hook_base, _hook_index, FD_OBJECT_GET(_HOOK_NAME_<_hook_index>));              \
    template <size_t Index>                                                                           \
    _FN_RET_ _HOOK_NAME_<Index>::callback(__VA_ARGS__)

#define FD_HOOK(_TARGET_FN_, _FN_TYPE_, _FN_RET_, ...) _FD_HOOK(_TARGET_FN_, _fd_hook, _FN_TYPE_, _FN_RET_, __VA_ARGS__)
