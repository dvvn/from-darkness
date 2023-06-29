#pragma once

#include "basic_pattern.h"

#include "fd/tool/array.h"
#include "fd/tool/span.h"
#include "fd/tool/vector.h"

#include <boost/hana/tuple.hpp>

#include <algorithm>
#include <charconv>
#include <ranges>

namespace fd
{
namespace detail
{
template <size_t S>
struct pattern_string
{
    char buff[S - 1];

    template <size_t... I>
    consteval pattern_string(char const *buff, std::index_sequence<I...>)
        : buff{buff[I]...}
    {
    }

    consteval pattern_string(char const (&buff)[S])
        : pattern_string(buff, std::make_index_sequence<S - 1>())
    {
    }
};
#ifdef __cpp_lib_constexpr_vector
template <class C>
using dynamic_pattern_buffer = vector<C>;
#else
#error custom constexpr vector not implemented
#endif
} // namespace detail

class dynamic_pattern_segment final : public basic_pattern_segment
{
    using buffer_type = detail::dynamic_pattern_buffer<value_type>;

    buffer_type buffer_;
    size_type tail_;

  public:
    constexpr dynamic_pattern_segment()
        : tail_(0)
    {
    }

    constexpr char const *fill(char const *pattern, char const *pattern_end)
    {
        for (auto part : span(pattern, pattern_end) | std::views::lazy_split(' '))
        {
            auto it = part.begin();

            if (*it == '?')
            {
                ++tail_;
                continue;
            }

            auto part_begin = &*it;

            // next part detected
            if (tail_ != 0)
                return part_begin;

            auto part_end = part_begin + std::ranges::distance(part);

            value_type value = 0;

            auto result = std::from_chars(part_begin, part_end, value, 16);
            if (result.ec != std::errc())
                std::unreachable();

            buffer_.emplace_back(value);
        }

        return pattern_end;
    }

    constexpr std::pair<pointer, size_type> extract() const
    {
        return {buffer_.data(), tail_};
    }

    constexpr pointer data() const
    {
        return buffer_.data();
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

    constexpr size_type length() const
    {
        return buffer_.size();
    }

    constexpr size_type abs_length() const
    {
        return length() + tail_;
    }

  protected:
    size_type self_size() const override
    {
        return sizeof(dynamic_pattern_segment);
    }
};

class dynamic_pattern : public basic_pattern
{
    friend constexpr size_t size(dynamic_pattern const &p);

    using segment_type = dynamic_pattern_segment;
    using buffer_type  = detail::dynamic_pattern_buffer<segment_type>;

    buffer_type buffer_;

  public:
    constexpr dynamic_pattern(char const *pat, size_t pattern_length)
    {
        for (auto end = pat + pattern_length; pat != end;)
        {
            auto &segment = buffer_.emplace_back();
            pat           = segment.fill(pat, end);
        }
    }

    size_t size() const override
    {
        return buffer_.size();
    }

    constexpr size_t length() const
    {
        size_t ret = 0;
        for (auto &s : buffer_)
            ret += s.length();
        return ret;
    }

    /*size_t abs_length() const override
    {
        auto rng = buffer_ | std::views::transform(&segment_type::abs_length);
        return std::accumulate(rng.begin(), rng.end(), 0);
    }*/

    iterator begin() const override
    {
        return buffer_.data();
    }

    iterator end() const override
    {
        return buffer_.data() + buffer_.size();
    }

    constexpr dynamic_pattern_segment const &operator[](ptrdiff_t index) const
    {
        return buffer_[index];
    }
};

constexpr size_t size(dynamic_pattern const &p)
{
    return p.buffer_.size();
}

namespace detail
{
template <size_t S>
constexpr dynamic_pattern make_pattern(char const (&pat)[S])
{
    return {pat, S - 1};
}
} // namespace detail

template <size_t Length>
requires(Length != 0)
class pattern_segment final : public basic_pattern_segment
{
    array<value_type, Length> bytes_;
    size_type tail_;

    template <size_t... I>
    constexpr pattern_segment(pointer bytes, size_type tail, std::index_sequence<I...>)
        : bytes_{bytes[I]...}
        , tail_(tail)
    {
    }

  public:
    constexpr pattern_segment(pointer bytes, size_type tail = 0)
        : pattern_segment(bytes, tail, std::make_index_sequence<Length>())
    {
    }

    constexpr pattern_segment(std::pair<pointer, size_type> info)
        : pattern_segment(info.first, info.second, std::make_index_sequence<Length>())
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

    constexpr size_type length() const
    {
        return Length;
    }

    constexpr size_type abs_length() const
    {
        return length() + tail_;
    }

  protected:
    size_type self_size() const override
    {
        return sizeof(pattern_segment);
    }
};

template <size_t... SegmentLength>
requires(sizeof...(SegmentLength) != 0)
class pattern : public basic_pattern
{
    static constexpr bool tuple_store_elements_as_is()
    {
        boost::hana::tuple<char, char> tpl(10, 8);
        return &at_c<0>(tpl) + 1 == &at_c<1>(tpl);
    }

    // non-std tuple because std's one store elements back-to-front
    using buffer_type = boost::hana::tuple<pattern_segment<SegmentLength>...>;
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

    size_t size() const override
    {
        return sizeof...(SegmentLength);
    }

    // size_t length() const override
    //{
    //     return (SegmentLength + ...);
    // }

    // size_t abs_length() const override
    //{
    //     return sum(transform(buffer_, []<size_t L>(pattern_segment<L> const &s) { return L /*+ s.tail()*/; }));
    // }
};

namespace detail
{
template <pattern_string Pattern>
constexpr auto make_pattern()
{
    // ReSharper disable once CppInconsistentNaming
#define make_hint() dynamic_pattern(Pattern.buff, std::size(Pattern.buff))

    constexpr auto convert_segments = []<size_t... I>(std::index_sequence<I...>) {
        auto hint = make_hint();
        return pattern<make_hint()[I].length()...>(hint[I].extract()...);
    };

    return convert_segments(std::make_index_sequence<size(make_hint())>());

#undef make_hint
}

template <pattern_string Pattern>
constexpr auto cached_pattern = make_pattern<Pattern>();
} // namespace detail

template <detail::pattern_string Pattern>
constexpr auto operator""_pat()
{
    return detail::cached_pattern<Pattern>;
    // return detail::make_pattern<Pattern>();
}

using detail::make_pattern;

} // namespace fd