﻿#pragma once

#include "basic_pattern.h"
#include "pattern_allocator.h"
#include "container/array.h"
#include "container/span.h"
#include "diagnostics/fatal.h"
#include "string/charconv.h"

#include <boost/hana/append.hpp>
#include <boost/hana/back.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>

namespace fd
{
namespace detail
{
template <pattern_size_type S>
struct pattern_string
{
    char buff[S];

    template <pattern_size_type... I>
    consteval pattern_string(char const* buff, std::index_sequence<I...>)
        : buff{buff[I]...}
    {
    }

    consteval pattern_string(char const* buff)
        : pattern_string(buff, std::make_index_sequence<S>())
    {
    }

    //--

    // ReSharper disable once CppMemberFunctionMayBeStatic
    constexpr pattern_size_type length() const
    {
        return S;
    }

    constexpr char const* begin() const
    {
        return std::begin(buff);
    }

    constexpr char const* end() const
    {
        return std::end(buff);
    }

    //--

    constexpr pattern_size_type bytes_count() const
    {
        return std::count(begin(), end(), ' ') + 1;
    }

    constexpr pattern_size_type segements_count() const
    {
        pattern_size_type count = 1;
        auto found              = false;

        for (auto c : buff)
        {
            if (c == '?')
            {
                found = true;
            }
            else if (found)
            {
                ++count;
                found = false;
            }
        }
        return count;
    }

    constexpr pattern_size_type longest_segment() const
    {
        pattern_size_type length         = 0;
        pattern_size_type current_length = 0;

        for (auto c : buff)
        {
            if (c == ' ')
            {
                ++current_length;
            }
            else if (c == '?' && length < current_length)
            {
                length         = current_length;
                current_length = 0;
            }
        }

        return length;
    }
};

template <size_t S>
pattern_string(char const (&)[S]) -> pattern_string<S - 1>;

struct pattern_segment_item
{
    uint8_t byte = -1;
    bool unknown = true;

    constexpr operator uint8_t() const
    {
        assert(!unknown);
        return byte;
    }
};
} // namespace detail

template <detail::pattern_size_type BytesCount>
struct pattern_segment
{
    array<uint8_t, BytesCount> known_bytes;
    detail::pattern_size_type unknown_bytes;

    template <typename It>
    constexpr pattern_segment(It known_from, It known_to, detail::pattern_size_type const unknown_bytes)
        : known_bytes()
        , unknown_bytes(unknown_bytes)
    {
        auto end = std::copy(known_from, known_to, known_bytes.begin());
        assert(known_bytes.end() == end);
    }

    constexpr detail::pattern_size_type size() const
    {
        return BytesCount;
    }

    detail::pattern_size_type whole_size() const
    {
        return size() + unknown_bytes;
    }
};

template <>
struct pattern_segment<1>
{
    uint8_t known_bytes;
    detail::pattern_size_type unknown_bytes;
};

template <>
struct pattern_segment<0>
{
    span</*uint8_t*/ char const> known_bytes;
    detail::pattern_size_type unknown_bytes;
};

template <detail::pattern_size_type BytesCount>
detail::pattern_size_type size(pattern_segment<BytesCount> const& segment)
{
    if constexpr (BytesCount == 1)
        return 1;
    else
        return size(segment.known_bytes);
}

template <detail::pattern_size_type BytesCount>
detail::pattern_size_type abs_size(pattern_segment<BytesCount> const& segment)
{
    return size(segment) + segment.unknown_bytes;
}

template <detail::pattern_size_type BytesCount>
auto begin(pattern_segment<BytesCount> const& segment)
{
    if constexpr (BytesCount == 1)
        return &segment.known_bytes;
    else
        return begin(segment.known_bytes);
}

template <detail::pattern_size_type BytesCount>
auto end(pattern_segment<BytesCount> const& segment)
{
    if constexpr (BytesCount == 1)
        return &segment.known_bytes + 1;
    else
        return end(segment.known_bytes);
}

template <detail::pattern_size_type BytesCount>
uint8_t first_byte(pattern_segment<BytesCount> const& segment)
{
    if constexpr (BytesCount == 1)
        return segment.known_bytes;
    else
        return (segment.known_bytes[0]);
}

template <detail::pattern_size_type... SegmentsBytesCount>
struct pattern
{
    boost::hana::tuple<pattern_segment<SegmentsBytesCount>...> bytes;

    constexpr pattern(pattern_segment<SegmentsBytesCount>... segment)
        : bytes(segment...)
    {
    }

    detail::pattern_size_type size() const
    {
        detail::pattern_size_type sum = 0;
        boost::hana::for_each(bytes, [&sum](auto& segment) {
            sum += abs_size(segment);
        });
        return sum;
    }
};

namespace detail
{
template <pattern_size_type BytesCount>
struct transformed_pattern : array<pattern_segment_item, BytesCount>
{
    constexpr transformed_pattern(span<char const> const pattern)
        : transformed_pattern()
    {
        using std::ranges::distance;
#ifdef _DEBUG
        using std::views::split;
#else
        constexpr auto split = std::views::lazy_split;
#endif
        auto splitted = pattern | split(' ');
        assert(distance(splitted) == BytesCount);

        std::ranges::for_each(splitted, [it = this->begin()](auto&& byte) mutable {
            using std::ranges::begin;
            using std::ranges::end;

            auto const curr = begin(byte);
            auto& curr_ref  = *curr;

            if (curr_ref == '?')
                it->unknown = true;
            else
            {
                auto const length = distance(byte);
                assert(length == 1 || length == 2);
                auto curr_ptr  = &curr_ref;
                auto const err = from_chars(curr_ptr, curr_ptr + length, it->byte, 16);
                assert(err.ec == std::errc{});
                it->unknown = false;
            }
            ++it;
        });
    }

    constexpr transformed_pattern() = default;

    constexpr size_t segments_count() const
    {
        size_t count = 0;

        auto const last = this->end();
        for (auto it = this->begin(); it != last;)
        {
            it = std::ranges::find(it, last, true, &pattern_segment_item::unknown);
            it = std::ranges::find(it, last, false, &pattern_segment_item::unknown);
            ++count;
        }

        return count;
    }

    struct segment_view
    {
        using iterator = typename array<pattern_segment_item, BytesCount>::const_iterator;

        iterator begin;
        iterator end;
        pattern_size_type unknown;

        constexpr pattern_size_type length() const
        {
            return std::distance(begin, end);
        }
    };

    constexpr segment_view segment(size_t skip = 0) const
    {
        auto const first = this->begin();
        auto const last  = this->end();
        for (auto it = first; it != last;)
        {
            auto const end  = std::ranges::find(it, last, true, &pattern_segment_item::unknown);
            auto const next = std::ranges::find(end, last, false, &pattern_segment_item::unknown);

            if (skip == 0)
                return {it, end, static_cast<pattern_size_type>(std::distance(end, next))};

            it = next;
            --skip;
        }

        return {first, last, skip == 0 ? 0 : BytesCount};
    }
};

template <size_t SegmentLength>
constexpr auto make_segment(auto tmp_segment)
{
    return pattern_segment<SegmentLength>(tmp_segment.begin, tmp_segment.end, tmp_segment.unknown);
}

template <pattern_string Str>
struct static_pattern
{
    static constexpr auto bytes = transformed_pattern<Str.bytes_count()>({Str.begin(), Str.end()});
};

constexpr auto test = static_pattern<"1 2 3 ? 4">::bytes;

template <pattern_string Str>
constexpr auto make_pattern()
{
    constexpr auto bytes_count = Str.bytes_count();
    constexpr transformed_pattern<bytes_count> bytes({Str.begin(), Str.end()});

    auto make_segment = []<size_t Skip>(std::in_place_index_t<Skip>) {
        constexpr auto tmp_segment    = bytes.segment(Skip);
        constexpr auto segment_length = tmp_segment.length();
        if constexpr (segment_length == 0)
            unreachable();
        else if constexpr (segment_length == 1)
            return pattern_segment<1>(*tmp_segment.begin, tmp_segment.unknown);
        else
            return pattern_segment<segment_length>(tmp_segment.begin, tmp_segment.end, tmp_segment.unknown);
    };
    constexpr auto segments_count = bytes.segments_count();
    return [&]<size_t... I>(std::index_sequence<I...>) {
        return pattern{make_segment(std::in_place_index<I>)...};
    }(std::make_index_sequence<segments_count>());
}
} // namespace detail

inline namespace literals
{
template <detail::pattern_string Pattern>
constexpr auto operator""_pat()
{
#ifndef _DEBUG
    constexpr
#endif
        auto result = detail::make_pattern<Pattern>();
    return result;
}
} // namespace literals

template <typename... Args>
constexpr auto make_pattern(Args const&... args)
{
    constexpr auto segments_count = ((std::same_as<Args, char> || std::constructible_from<span<char const>, Args const&>)+...);

    boost::hana::tuple<Args const&...> args_packed(args...);

    auto const make_segment2 = [&]<typename T>(T const& str, auto unknown) {
        if constexpr (std::is_bounded_array_v<T>)
            return pattern_segment<sizeof(T) - 1>(std::begin(str), std::end(str) - 1, unknown);
        else if constexpr (std::same_as<T, char>)
            return pattern_segment<1>(str, unknown);
        else
            return pattern_segment<0>(str, unknown);
    };

    auto const make_segment = [&]<size_t I>(std::in_place_index_t<I>) {
        constexpr auto index = I % 2 ? I + 1 : I;
        auto const& str      = boost::hana::at_c<index>(args_packed);
        auto const num       = boost::hana::at_c<index + 1>(args_packed);
        return make_segment2(str, num);
    };
    if constexpr (sizeof...(Args) % 2 == 0)
    {
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return pattern{make_segment(std::in_place_index<I>)...};
        }(std::make_index_sequence<segments_count>());
    }
    else
    {
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return pattern{make_segment(std::in_place_index<I>)..., make_segment2(boost::hana::back(args_packed), 0)};
        }(std::make_index_sequence<segments_count - 1>());
    }
}
} // namespace fd
