#pragma once

// #include <fd/string.h>
#include <fd/algorithm.h>
#ifdef _DEBUG
#include <fd/exception.h>
#endif

namespace fd
{
template <bool ExactSize, typename Chr, size_t Size>
struct chars_cache;

template <typename Chr, size_t Size>
struct chars_cache<true, Chr, Size>
{
    Chr charsBuff[Size];

    using value_type    = Chr;
    using pointer       = Chr*;
    using const_pointer = const Chr*;

    constexpr chars_cache(const_pointer strSource)
    {
        copy(strSource, Size, charsBuff);
#ifdef _DEBUG
        if (!equal(strSource, charsBuff, Size))
            abort();
#endif
    }

    constexpr const_pointer data() const
    {
        return charsBuff;
    }

    constexpr pointer data()
    {
        return charsBuff;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    constexpr size_t size() const
    {
        return Size;
    }

    constexpr const_pointer begin() const
    {
        return charsBuff;
    }

    constexpr const_pointer end() const
    {
        return charsBuff + size();
    }

    constexpr pointer begin()
    {
        return charsBuff;
    }

    constexpr pointer end()
    {
        return charsBuff + size();
    }

    /*constexpr strv_t view() const
    {
        return { begin(), size() };
    }

    constexpr operator strv_t() const
    {
        return view();
    }*/
};

template <typename Chr, size_t Size>
struct chars_cache<false, Chr, Size>
{
    Chr    charsBuff[Size];
    size_t charsCount;

    using value_type    = Chr;
    using pointer       = Chr*;
    using const_pointer = const Chr*;

    constexpr chars_cache()
        : charsBuff()
        , charsCount(0)
    {
    }

    constexpr chars_cache(const_pointer strSource, const size_t strSize = Size)
        : charsCount(strSize)
    {
#ifdef _DEBUG
        if (Size < strSize)
        {
            unload();
            charsCount = 0;
            return;
        }
#endif
        copy(strSource, strSize, charsBuff);
    }

    constexpr const_pointer data() const
    {
        return charsBuff;
    }

    constexpr pointer data()
    {
        return charsBuff;
    }

    constexpr size_t size() const
    {
        return charsCount;
    }

    constexpr const_pointer begin() const
    {
        return charsBuff;
    }

    constexpr const_pointer end() const
    {
        return charsBuff + charsCount;
    }

    constexpr pointer begin()
    {
        return charsBuff;
    }

    constexpr pointer end()
    {
        return charsBuff + charsCount;
    }

    /*constexpr strv_t view() const
    {
        return { charsBuff, charsCount };
    }

    constexpr operator strv_t() const
    {
        return { charsBuff, charsCount };
    }*/
};

template <bool ExactSize, typename Chr, size_t Size>
constexpr bool operator==(const chars_cache<ExactSize, Chr, Size>& left, const Chr* right)
{
    return equal(left, right) && right[Size] == '\0';
}

template <bool ExactSize, typename Chr, size_t Size>
constexpr bool operator==(const Chr* left, const chars_cache<ExactSize, Chr, Size>& right)
{
    return right == left;
}

#if 0
template <bool ExactSize, typename Chr, size_t Size>
constexpr bool operator==(const chars_cache<ExactSize, Chr, Size>& left, const basic_string_view<Chr> right)
{
    return basic_string_view<Chr>(left) == right;
}

template <bool ExactSize, typename Chr, size_t Size>
constexpr bool operator==(const chars_cache<ExactSize, Chr, Size>& left, const Chr* right)
{
    return basic_string_view<Chr>(left) == right;
}

template <bool ExactSize, typename Chr, size_t Size>
constexpr bool operator==(const basic_string_view<Chr> left, const chars_cache<ExactSize, Chr, Size> right)
{
    return right == left;
}

template <bool ExactSize, typename Chr, size_t Size>
constexpr bool operator==(const Chr* left, const chars_cache<ExactSize, Chr, Size> right)
{
    return right == left;
}
#endif

template <typename Chr, size_t Size>
chars_cache(const Chr (&arr)[Size]) -> chars_cache<true, Chr, Size - 1>;
}