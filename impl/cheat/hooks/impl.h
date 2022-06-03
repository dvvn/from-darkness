#pragma once

#include <cheat/core/object.h>

#include <nstd/runtime_assert.h>

#include <array>
#include <string_view>

import cheat.hook;

#define FN_member
#define FN_static static

#define IS_STATIC_member false
#define IS_STATIC_static true

// clang-format off
#define CONCAT_NAMESPACE(_L_, _R_) #_L_"::"#_R_
// clang-format on

#define CHEAT_HOOK_BEGIN(_HOOK_SOURCE_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)            \
    using namespace cheat;                                                                \
    using namespace hooks;                                                                \
    using namespace _HOOK_SOURCE_;                                                        \
    struct _HOOK_NAME_##_impl final : hook, hook_instance_##_FN_TYPE_<_HOOK_NAME_##_impl> \
    {                                                                                     \
        bool is_static() const override                                                   \
        {                                                                                 \
            return IS_STATIC_##_FN_TYPE_;                                                 \
        }                                                                                 \
        std::string_view name() const override                                            \
        {                                                                                 \
            return CONCAT_NAMESPACE(_HOOK_SOURCE_, _HOOK_NAME_);                          \
        }                                                                                 \
        void init() override;                                                             \
        FN_##_FN_TYPE_ _FN_RET_ callback(__VA_ARGS__);

#define CHEAT_HOOK_END(_HOOK_NAME_)                        \
    }                                                      \
    ; /*_HOOK_NAME_ -> one_instance_t::operator size_t()*/ \
    CHEAT_OBJECT_BIND(base, _HOOK_NAME_, _HOOK_NAME_##_impl)

#define CHEAT_HOOK_BODY(_HOOK_SOURCE_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)      \
    CHEAT_HOOK_BEGIN(_HOOK_SOURCE_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, __VA_ARGS__) \
    CHEAT_HOOK_END(_HOOK_NAME_)

#define CHEAT_HOOK_INIT(_HOOK_NAME_) void _HOOK_NAME_##_impl::init()
#define CHEAT_HOOK_CALLBACK(_HOOK_NAME_, _FN_RET_, ...) _FN_RET_ _HOOK_NAME_##_impl::callback(__VA_ARGS__)

//---

template <class T>
concept have_find_method = requires
{
    T::find("hello");
};

template <class E = void>
constexpr bool _Extractor_validate(const std::string_view file_name, size_t start = 0, size_t end = 0)
{
    if (start * end == 0 && start != end)
    {
        if (!std::is_constant_evaluated())
            runtime_assert("Incorrect start and end!");
        return false;
    }
    if constexpr (have_find_method<E>)
    {
        if (start == 0)
        {
            const auto [start1, end1] = E::find(file_name);
            start = start1;
            end = end1;
        }
    }
#ifdef _DEBUG
    if (!std::is_constant_evaluated())
    {
        runtime_assert(start != file_name.npos);
        runtime_assert(end != file_name.npos);
        runtime_assert(start < end);
    }
    else
#endif
    {
        if (start == file_name.npos)
            return false;
        if (end == file_name.npos)
            return false;
        if (start >= end)
            return false;
    }
    return true;
}

class hook_index_extractor
{
    static constexpr size_t char_to_num(const char chr)
    {
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
            runtime_assert_unreachable("Incorrect character");
        }
    }

    size_t index_ = 0;

  public:
    static constexpr auto find(const std::string_view file_name)
    {
        auto start = file_name.rfind('_');
        if (start != file_name.npos)
            ++start;
        const auto end = file_name.find('.', start);
        return std::pair(start, end);
    }

    constexpr hook_index_extractor(const std::string_view file_name, const bool validate = false)
    {
        const auto [start, end] = find(file_name);
        if (validate && !_Extractor_validate(file_name, start, end))
            index_ = file_name.npos;
        else
        {
            const auto size = end - start;
            for (const auto chr : file_name.substr(start, size))
                index_ = char_to_num(chr) + index_ * 10;
        }
    }

    constexpr operator size_t() const
    {
        return index_;
    }
};

constexpr auto _Corrent_path(const std::string_view path)
{
    // std::source_location::current().file_name() contains '\' and '/' in same time
    std::string buff;
    buff.reserve(path.size());
    for (const auto c : path)
        buff += (c == '\\' ? '/' : c);
    return buff;
}

struct hook_name_extractor : std::string_view
{
    static constexpr auto find(const std::string_view file_name)
    {
        auto start = _Corrent_path(file_name).rfind('/');
        if (start != file_name.npos)
            ++start;
        const auto end = file_name.find('_', start);
        return std::pair(start, end);
    }

    static constexpr std::string_view get(const std::string_view file_name, const bool validate)
    {
        const auto [start, end] = find(file_name);
        if (validate && !_Extractor_validate(file_name, start, end))
            return {nullptr, 0u};
        const auto size = end - start;
        return file_name.substr(start, size);
    }

    constexpr hook_name_extractor(const std::string_view file_or_name, const bool custom, const bool validate = true)
        : std::string_view(custom ? file_or_name : get(file_or_name, validate))
    {
    }
};

struct hook_group_name_extractor : std::string_view
{
    static constexpr auto find(const std::string_view file_name)
    {
        const auto file_name_correct = _Corrent_path(file_name);
        const auto end = file_name_correct.rfind('/');
        auto start = file_name_correct.substr(0, end).rfind('/');
        if (start != file_name.npos)
            ++start;
        return std::pair(start, end);
    }

    static constexpr std::string_view get(const std::string_view file_name, const bool validate)
    {
        const auto [start, end] = find(file_name);
        if (validate && !_Extractor_validate(file_name, start, end))
            return {nullptr, 0u};
        const auto size = end - start;
        return file_name.substr(start, size);
    }

    constexpr hook_group_name_extractor(const std::string_view file_or_name, const bool custom, const bool validate = true)
        : std::string_view(custom ? file_or_name : get(file_or_name, validate))
    {
    }
};

template <size_t BuffSize>
constexpr auto _Hook_name(const std::string_view name, const std::string_view group)
{
    std::array<char, BuffSize + 2 + 1> buff;

    const auto buff_group_bg = buff.begin();
    const auto buff_namespace_bg = buff_group_bg + group.size();
    const auto buff_name_bg = buff_namespace_bg + 2;

    std::copy(group.begin(), group.end(), buff_group_bg);
    std::fill_n(buff_namespace_bg, 2, ':');
    std::copy(name.begin(), name.end(), buff_name_bg);
    buff.back() = '\0';
    return buff;
}

#define CHEAT_HOOK_NAME(_HOOK_NAME_)                                \
    [] {                                                            \
        constexpr hook_name_extractor name(_HOOK_NAME_, true);      \
        constexpr hook_group_name_extractor group(__FILE__, false); \
        return _Hook_name<name.size() + group.size()>(name, group); \
    }

#define FN_member
#define FN_static static

#define IS_STATIC_member false
#define IS_STATIC_static true

#define CHEAT_HOOK(_TARGET_FN_, _HOOK_NAME_, _FN_TYPE_, _FN_RET_, ...)                                       \
    struct _HOOK_NAME_##_impl final : cheat::hook_impl, cheat::hook_instance_##_FN_TYPE_<_HOOK_NAME_##_impl> \
    {                                                                                                        \
        bool is_static() const override                                                                      \
        {                                                                                                    \
            return IS_STATIC_##_FN_TYPE_;                                                                    \
        }                                                                                                    \
        std::string_view name() const override                                                               \
        {                                                                                                    \
            static constexpr auto debug_name = CHEAT_HOOK_NAME(#_HOOK_NAME_)();                              \
            return {debug_name.data(), debug_name.size() - 1};                                               \
        }                                                                                                    \
        void init() override                                                                                 \
        {                                                                                                    \
            hook::init(_TARGET_FN_, &_HOOK_NAME_##_impl::callback);                                          \
        }                                                                                                    \
        FN_##_FN_TYPE_ _FN_RET_ callback(__VA_ARGS__);                                                       \
    };                                                                                                       \
    CHEAT_OBJECT_BIND(cheat::hook_base, hook_index_extractor(__FILE__), _HOOK_NAME_##_impl);                 \
    _FN_RET_ _HOOK_NAME_##_impl::callback(__VA_ARGS__)
