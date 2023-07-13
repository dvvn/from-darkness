#pragma once

#include <algorithm>

namespace fd
{
template <size_t S>
struct library_tag
{
    static constexpr size_t static_length()
    {
        return S - 1;
    }

    char tag[static_length()];
    using pointer = char const *;

    consteval library_tag(char const (&name)[S])
        : tag()
    {
        std::copy(name, name + static_length(), tag);
    }

    constexpr pointer data() const
    {
        return tag;
    }

    constexpr size_t length() const
    {
        return static_length();
    }

    constexpr pointer begin() const
    {
        return data();
    }

    constexpr pointer end() const
    {
        return data() + length();
    }

    /* constexpr operator string_view() const
     {
         return {data(), length()};
     }*/

    template <typename C, size_t ExtS>
    void add_extension(C (&buffer)[static_length() + ExtS - 1], char const (&ext)[ExtS]) const
    {
        std::copy(ext, ext + ExtS - 1, std::copy(begin(), end(), buffer));
    }
};

template <size_t S>
constexpr size_t size(library_tag<S> const &tag)
{
    return tag.length();
}

template <class T>
struct library_member;
//{
//    static constexpr library_tag name = "XXXXXXXXX";
//};
}