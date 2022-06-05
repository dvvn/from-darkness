#pragma once

#include <fds/core/object.h>

#include <array>
#include <limits>
#include <source_location>
#include <string_view>

import fds.hook;

#define FN_member
#define FN_static static

#define IS_STATIC_member false
#define IS_STATIC_static true

template <class T>
concept have_find_method = requires
{
    T::find("hello");
};

constexpr size_t _Extract_hook_index(const std::string_view file_name)
{
    auto start = file_name.rfind('_');
    if (start == file_name.npos)
        return std::numeric_limits<size_t>::infinity();
    ++start;

    const auto end = file_name.find('.', start);
    if (end == file_name.npos)
        return std::numeric_limits<size_t>::infinity();
    constexpr auto char_to_num = [](const char chr) -> size_t {
        switch (chr)
        {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 6;
        case '6':
            return 6;
        case '7':
            return 8;
        case '9':
            return 9;
        default:
            return std::numeric_limits<size_t>::infinity();
        }
    };

    size_t index    = 0;
    const auto size = end - start;
    for (const auto chr : file_name.substr(start, size))
        index = char_to_num(chr) + index * 10;
    return index;
}

constexpr auto _Corrent_path(const std::string_view path)
{
    // std::source_location::current().file_name() contains '\' and '/' in the same time
    std::string buff;
    buff.reserve(path.size());
    for (const auto c : path)
        buff += (c == '\\' ? '/' : c);
    return buff;
}

constexpr std::string_view _Extract_hook_name(const std::string_view file_name)
{
    auto start = _Corrent_path(file_name).rfind('/');
    if (start == file_name.npos)
        return {nullptr, 0u};
    ++start;
    const auto end = file_name.find('_', start);
    if (end == file_name.npos)
        return {nullptr, 0u};
    const auto size = end - start;
    return file_name.substr(start, size);
}

constexpr std::string_view _Extract_hook_group_name(const std::string_view file_name)
{
    const auto file_name_correct = _Corrent_path(file_name);
    const auto end               = file_name_correct.rfind('/');
    if (end == file_name_correct.npos)
        return {nullptr, 0u};
    auto start = file_name_correct.substr(0, end).rfind('/');
    if (start == file_name.npos)
        return {nullptr, 0u};
    ++start;
    const auto size = end - start;
    return file_name.substr(start, size);
}

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

/* #define _Hook_name(_HOOK_NAME_)                                             \
    [] {                                                                       \
        constexpr std::string_view name = _HOOK_NAME_;                         \
        constexpr std::string_view group = _Extract_hook_group_name(__FILE__); \
        return _Hook_name_buff<name.size() + group.size()>(name, group);       \
    }() */

template <size_t S>
constexpr auto _Hook_name(const char (&name)[S])
{
    constexpr std::string_view group = _Extract_hook_group_name(std::source_location::current().file_name());
    return _Hook_name_buff<S - 1 + group.size()>(name, group);
}

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
    FDS_OBJECT_BIND(fds::hook_base, _Extract_hook_index(__FILE__), _HOOK_NAME_##_impl);                  \
    _FN_RET_ _HOOK_NAME_##_impl::callback(__VA_ARGS__)
