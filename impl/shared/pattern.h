﻿#pragma once

#include "basic_pattern.h"
#include "container/array.h"
#include "container/span.h"
#include "container/vector/dynamic.h"
#include "diagnostics/fatal.h"
#include "string/char.h"
#include "string/charconv.h"
#include "string/view.h"

#include <boost/hana/append.hpp>
#include <boost/hana/sum.hpp>
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
    char buff[S - 1];

    template <pattern_size_type... I>
    consteval pattern_string(char const *buff, std::index_sequence<I...>)
        : buff{buff[I]...}
    {
    }

    consteval pattern_string(char const (&buff)[S])
        : pattern_string(buff, std::make_index_sequence<S - 1>())
    {
    }
};

template <typename T>
struct dynamic_segment_allocator : std::allocator<T>
{
    using size_type       = pattern_size_type;
    using difference_type = pattern_difference_type;
};

using packed_pattern_segment = std::pair<pattern_segment_pointer, pattern_size_type>;

constexpr uint8_t validate_pattern_char(char c)
{
    return c == '?' ? 2 : isxdigit(c) ? 1 : 0;
}

struct bad_pattern
{
    static void no_data();
    static void front_unknown();
    static void unsupported_char();
    static void bad_character();
    static void incompatible_chars();
    static void bad_character2();
};

template <typename T>
constexpr void validate_pattern(char const *pattern, pattern_size_type length, T &&cache)
{
    if constexpr (!std::ranges::range<T>)
    {
        validate_pattern(pattern, length, span(pattern, length) | std::views::lazy_split(' '));
    }
    else
    {
        using std::ranges::begin;
        using std::ranges::distance;
        using std::ranges::empty;
        using std::ranges::for_each;
        using std::ranges::next;

        if (empty(cache))
            return bad_pattern::no_data();

        if (pattern[0] == '?')
            return bad_pattern::front_unknown();

        for_each(cache, [](auto &&part) {
            switch (distance(part))
            {
            case 1: {
                if (!validate_pattern_char(*begin(part)))
                    return bad_pattern::bad_character();
                break;
            }
            case 2: {
                auto it  = begin(part);
                auto tmp = validate_pattern_char(*it);
                if (!tmp)
                    return bad_pattern::bad_character();
                auto tmp2 = validate_pattern_char(*next(it));
                if (!tmp2)
                    return bad_pattern::bad_character2();
                if (tmp != tmp2)
                    return bad_pattern::incompatible_chars();
                break;
            }
            default: {
                return bad_pattern::unsupported_char();
            }
            }
        });
    }
}

template <typename It>
constexpr pattern_size_type pattern_to_segments(It segment,
                                                char const *raw_pattern,
                                                pattern_size_type length,
                                                pattern_size_type segments_ignore)
{
#ifdef _DEBUG
    constexpr auto split = std::views::lazy_split;
#else
    using std::views::split;
#endif
    using std::ranges::begin;
    using std::ranges::distance;
    using std::ranges::for_each;
    using std::views::drop;

    auto view = span(raw_pattern, length) | split(' ') | drop(segments_ignore);
#ifdef _DEBUG
    validate_pattern(raw_pattern, length, view);
#endif

    pattern_size_type segments_count = 1;

    for_each(view, [&](auto &&part) {
        auto it = begin(part);

        if (*it == '?')
        {
            ++segment->tail;
            return;
        }

        if (segment->tail != 0)
        {
            ++segment;
            ++segments_count;
        }

        auto part_begin = &*it;
        auto part_end   = part_begin + distance(part);

        pattern_segment_value_type value;

        [[maybe_unused]] //
        auto result = from_chars(part_begin, part_end, value, 16);
#ifndef _DEBUG
        if (result.ec != std::errc())
            unreachable();
#endif
        segment->emplace_back(value);
    });

    return segments_count;
}

template <pattern_size_type Length>
struct dirty_pattern
{
    struct segment;

    using pointer   = segment const *;
    using iterator  = pointer;
    using reference = segment const &;

    struct segment
    {
        pattern_segment_value_type buff[Length];
        pattern_size_type tail;

        pattern_size_type length;

        constexpr segment()
            : buff()
            , tail(0)
            , length(0)
        {
        }

        constexpr void emplace_back(pattern_segment_value_type byte)
        {
            buff[length] = byte;
            ++length;
        }

        constexpr packed_pattern_segment extract() const
        {
            return {buff, tail};
        }
    };

  private:
    segment segments_[Length];
    pattern_size_type segments_count_;

  public:
    constexpr dirty_pattern(char const (&pattern)[Length], pattern_size_type ignore = 0)
        : segments_()
        , segments_count_(
              pattern_to_segments(std::begin(segments_), pattern, Length - (*std::rbegin(pattern) == '\0'), ignore))
    {
    }

    constexpr iterator begin() const
    {
        return std::begin(segments_);
    }

    constexpr iterator end() const
    {
        return begin() + count();
    }

    constexpr pattern_size_type count() const
    {
        return segments_count_;
    }

    constexpr reference operator[](pattern_difference_type idx) const
    {
        assert(idx < count());
        return segments_[idx];
    }
};

class dynamic_pattern_segment final : public basic_pattern_segment
{
    using buffer_type = vector_ex2<pattern_segment_value_type, dynamic_segment_allocator>;

    buffer_type buffer_;
    size_type tail_;

  public:
    constexpr dynamic_pattern_segment()
        : tail_(0)
    {
    }

    pointer begin() const override
    {
        return buffer_.data();
    }

    pointer end() const override
    {
        return buffer_.data() + buffer_.size();
    }

    size_type tail() const override
    {
        return tail_;
    }

  protected:
    size_type self_size() const override
    {
        return sizeof(dynamic_pattern_segment);
    }
};

//---

template <pattern_size_type Length>
class pattern_segment final : public basic_pattern_segment
{
    array<value_type, Length> bytes_;
    size_type tail_;

    template <size_type... I>
    constexpr pattern_segment(pointer bytes, size_type tail, std::integer_sequence<size_type, I...>)
        : bytes_{bytes[I]...}
        , tail_(tail)
    {
    }

  public:
    constexpr pattern_segment(pointer bytes, size_type tail = 0)
        : pattern_segment(bytes, tail, std::make_integer_sequence<size_type, Length>())
    {
    }

    constexpr pattern_segment(packed_pattern_segment info)
        : pattern_segment(info.first, info.second, std::make_integer_sequence<size_type, Length>())
    {
    }

    pointer begin() const override
    {
        return bytes_.data();
    }

    pointer end() const override
    {
        return begin() + Length;
    }

    size_type tail() const override
    {
        return tail_;
    }

  protected:
    size_type self_size() const override
    {
        return sizeof(pattern_segment);
    }
};

template <>
class pattern_segment<0> final : public basic_pattern_segment
{
    pointer bytes_;
    size_type length_;
    size_type tail_;

  public:
    pattern_segment(pointer bytes, size_type length, size_type tail = 0)
        : bytes_(bytes)
        , length_(length)
        , tail_(tail)
    {
    }

    pointer begin() const override
    {
        return bytes_;
    }

    pointer end() const override
    {
        return begin() + length_;
    }

    size_type tail() const override
    {
        return tail_;
    }

  protected:
    size_type self_size() const override
    {
        return sizeof(pattern_segment);
    }
};
} // namespace detail

class dynamic_pattern final : public basic_pattern
{
    using segment_type = detail::dynamic_pattern_segment;
    using buffer_type  = vector_ex2<segment_type, detail::dynamic_segment_allocator>;

    buffer_type buffer_;

  public:
    constexpr dynamic_pattern(char const *pat, size_type pattern_length)
    {
    }

    size_type segments() const override
    {
        return buffer_.size();
    }

    iterator begin() const override
    {
        return buffer_.data();
    }

    iterator end() const override
    {
        return buffer_.data() + buffer_.size();
    }
};

template <detail::pattern_size_type... SegmentLength>
class pattern final : public basic_pattern
{
    static constexpr bool tuple_store_elements_as_is()
    {
        boost::hana::tuple<char, char> tpl(10, 8);
        return &at_c<0>(tpl) + 1 == &at_c<1>(tpl);
    }

    // non-std tuple because std's one store elements back-to-front
    using buffer_type = boost::hana::tuple<detail::pattern_segment<SegmentLength>...>;
    buffer_type buffer_;

  public:
    constexpr pattern(auto... info)
        : buffer_(info...)
    {
        static_assert(tuple_store_elements_as_is());
    }

    iterator begin() const override
    {
        return &at_c<0>(buffer_);
    }

    iterator end() const override
    {
        return &at_c<sizeof...(SegmentLength) - 1>(buffer_) + 1;
        // return reinterpret_cast<segment>(std::next(&buffer_));
    }

    size_type segments() const override
    {
        return sizeof...(SegmentLength);
    }

    /*pattern_size_type length() const override
    {
        if constexpr (((SegmentLength == 0) || ...))
            return boost::hana::sum<pattern_size_type>(boost::hana::transform(buffer_, [](auto &p) { return p.length();
    })); else return (SegmentLength + ...);
    }*/

    // pattern_size_type abs_length() const override
    //{
    //     return sum(transform(buffer_, []<pattern_size_type L>(pattern_segment<L> const &s) { return L /*+ s.tail()*/;
    //     }));
    // }
};

// template <pattern_size_type... S>
// pattern(std::pair<pattern_string<S>, ptrdiff_t> const &...) -> pattern<S...>;

namespace detail
{
template <pattern_size_type S>
constexpr dynamic_pattern make_pattern(char const (&pat)[S])
{
    return {pat, S - 1};
}

template <pattern_string Pattern>
constexpr auto make_pattern()
{
    constexpr dirty_pattern pat(Pattern.buff);
    constexpr auto convert_segments = []<pattern_size_type... I>(std::integer_sequence<pattern_size_type, I...>) {
        return pattern<pat[I].length...>(pat[I].extract()...);
    };
    return convert_segments(std::make_integer_sequence<pattern_size_type, pat.count()>());
}

template <pattern_size_type S>
struct pattern_segment_info
{
    using pointer   = pattern_segment_pointer;
    using size_type = pattern_size_type;

    pointer bytes;
    static constexpr size_type length = S - 1;
    size_type tail;

    pattern_segment_info(char const *bytes, size_type tail = 0)
        : bytes(reinterpret_cast<pointer>(bytes))
        , tail(tail)
    {
    }

    pattern_segment<length> get() const
    {
        return {bytes, tail};
    }
};

template <pattern_size_type S>
pattern_segment_info(char const (&)[S], pattern_size_type = 0) -> pattern_segment_info<S>;

template <>
struct pattern_segment_info<0>
{
    using pointer   = pattern_segment_pointer;
    using size_type = pattern_size_type;

    pointer bytes;
    size_type length;
    size_type tail;

    pattern_segment_info(char const *bytes, size_type length, size_type tail = 0)
        : bytes(reinterpret_cast<pointer>(bytes))
        , length(length)
        , tail(tail)
    {
    }

    pattern_segment_info(string_view bytes, size_type tail = 0)
        : pattern_segment_info(bytes.data(), static_cast<size_type>(bytes.length()), tail)
    {
        assert(bytes.length() < std::numeric_limits<size_type>::max());
    }

    pattern_segment<0> get() const
    {
        return pattern_segment<0>{bytes, length, tail};
    }
};

// template <pattern_size_type S>
pattern_segment_info(std::type_identity_t<string_view>, auto...) -> pattern_segment_info<0>;

template <class T, typename A1>
constexpr auto pattern_args_to_segments(T &&tpl, A1 &arg1)
{
    return boost::hana::append(tpl, pattern_segment_info(arg1));
}

template <class T, typename A1, typename A2, typename... Args>
constexpr auto pattern_args_to_segments(T &&tpl, A1 &arg1, A2 &arg2, Args &...args)
{
    if constexpr (std::integral<A2>)
    {
        auto new_tpl = boost::hana::append(tpl, pattern_segment_info(arg1, arg2));
        if constexpr (sizeof...(Args) == 0)
            return new_tpl;
        else
            return pattern_args_to_segments(new_tpl, args...);
    }
    else
    {
        auto new_tpl = boost::hana::append(tpl, pattern_segment_info(arg1, 0));
        return pattern_args_to_segments(new_tpl, arg2, args...);
    }
}

template <typename... T>
constexpr auto make_pattern(T const &...args) requires(sizeof...(T) > 1)
{
    return boost::hana::unpack(detail::pattern_args_to_segments(boost::hana::tuple(), args...),
                               []<pattern_size_type... S>(pattern_segment_info<S>... segment) {
                                   return pattern < S ? S - 1 : 0 ... > (segment.get()...);
                               });
}

template <pattern_string Pattern>
constexpr auto cached_pattern = make_pattern<Pattern>();
} // namespace detail

inline namespace literals
{
template <detail::pattern_string Pattern>
constexpr auto operator""_pat()
{
    if (std::is_constant_evaluated())
        return detail::make_pattern<Pattern>();
    else
        return detail::cached_pattern<Pattern>;
}
} // namespace literals

using detail::make_pattern;

} // namespace fd