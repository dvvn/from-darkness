#pragma once

#include <fd/object.h>

#define _FD_CALLBACK(_CLASS_, _NAME_, _ARG_) FD_OBJECT(_NAME_, _CLASS_<_ARG_>, FD_UNIQUE_INDEX)
#define _FD_CALLBACK_EX(_CLASS_, _NAME_, ...) \
    namespace callbacks                       \
    {                                         \
        using _NAME_ = _CLASS_<__VA_ARGS__>;  \
    }                                         \
    FD_OBJECT(_NAME_, callbacks::_NAME_, FD_UNIQUE_INDEX)
#define FD_CALLBACK_SELECTOR(_CLASS_, _NAME_, _ARG1_, ...) _FD_CALLBACK##__VA_OPT__(_EX)(_CLASS_, _NAME_, _ARG1_, ##__VA_ARGS__)

constexpr char _Get_char_after_dot(const char* full_path, const size_t size)
{
    for (auto i = size - 1; i > 0; --i)
    {
        if (full_path[i] == '.')
            return full_path[i + 1];
    }
    return 0;
}

template <size_t S>
constexpr bool _Is_header_file(const char (&full_path)[S])
{
    switch (_Get_char_after_dot(full_path, S))
    {
    case 'h': //.h**
#if __cplusplus >= 202002L
    case 'i': //.i**
#endif
        return true;
    default:
        return false;
    }
}

template <size_t S>
constexpr bool _Is_source_file(const char (&full_path)[S])
{
    return _Get_char_after_dot(full_path, S) == 'c'; //.c**
}
