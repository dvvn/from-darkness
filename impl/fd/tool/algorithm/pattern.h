#pragma once

#include "basic_pattern.h"

#include "fd/tool/span.h"
#include "fd/tool/vector.h"

#include <boost/hana/tuple.hpp>

#include <algorithm>
#include <charconv>
#include <ranges>

namespace fd
{
namespace impl
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

class dynamic_pattern_segment final : public basic_pattern_segment
{
    using buffer_type = dynamic_pattern_buffer<value_type>;

    buffer_type buffer_;
    size_type gap_;

  public:
    constexpr dynamic_pattern_segment(char const *pattern, char const *pattern_end)
        : gap_(0)
    {
        /*auto end = pattern + length;
        for (; pattern != end; ++pattern)
        {
            auto c = *pattern;

        }*/

        for (auto part : span(pattern, pattern_end) | std::views::lazy_split(' '))
        {
            auto it = part.begin();

            if (*it == '?')
            {
                ++gap_;
                continue;
            }

            // next part detected
            if (gap_ != 0)
                break;

            auto part_begin = &*it;
            auto part_end   = part_begin + std::ranges::distance(part);

            value_type value = 0;

            auto result = std::from_chars(part_begin, part_end, value, 16);
            if (result.ec != std::errc())
                std::unreachable();

            buffer_.emplace_back(value);
        }
    }

    constexpr size_type length() const
    {
        return buffer_.size();
    }

    constexpr size_type abs_length() const
    {
        return length() + gap_;
    }

    constexpr std::pair<pointer, size_type> extract() const
    {
        return {buffer_.data(), gap_};
    }

    pointer begin() const override
    {
        return buffer_.data();
    }

    pointer end() const override
    {
        return buffer_.data() + buffer_.size();
    }

    size_type gap() const override
    {
        return gap_;
    }

  protected:
    size_t size() const override
    {
        return sizeof(dynamic_pattern_segment);
    }
};

class dynamic_pattern : public basic_pattern
{
    using buffer_type = dynamic_pattern_buffer<dynamic_pattern_segment>;

    buffer_type buffer_;

  public:
    constexpr dynamic_pattern(char const *pat, size_t length)
    {
        for (auto end = pat + length;;)
        {
            auto &segment = buffer_.emplace_back(pat, end);
            //"1 2 ?? 4 A ?": abs_length == 6, spaces count == 5. 6*2-1 == 6+5
            pat += segment.abs_length() * 2 - 1;

            for (;;)
            {
                if (pat == end)
                    return;
                if (*pat == '?' || *pat == ' ')
                    ++pat;
                else
                    break;
            }
        }
    }

    constexpr size_t length() const
    {
        return buffer_.size();
    }

    constexpr dynamic_pattern_segment const &operator[](ptrdiff_t index) const
    {
        return buffer_[index];
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
} // namespace impl

template <size_t S>
constexpr impl::dynamic_pattern make_pattern(char const (&pat)[S])
{
    return {pat, S - 1};
}

template <size_t Length>
class pattern_segment final : public basic_pattern_segment
{
    value_type bytes_[Length];
    size_type gap_;

    template <size_t... I>
    constexpr pattern_segment(pointer bytes, size_type gap, std::index_sequence<I...>)
        : bytes_{bytes[I]...}
        , gap_(gap)
    {
    }

  public:
    constexpr pattern_segment(pointer bytes, size_type gap = 0)
        : pattern_segment(bytes, gap, std::make_index_sequence<Length>())
    {
    }

    constexpr pattern_segment(std::pair<pointer, size_type> info)
        : pattern_segment(info.first, info.second, std::make_index_sequence<Length>())
    {
    }

    pointer begin() const override
    {
        return bytes_;
    }

    pointer end() const override
    {
        return bytes_ + Length;
    }

    size_type gap() const override
    {
        return gap_;
    }

  protected:
    size_t size() const override
    {
        return sizeof(pattern_segment);
    }
};

template <size_t... SegmentLength>
class pattern : public basic_pattern
{
    static constexpr bool tuple_store_elements_as_is()
    {
        boost::hana::tuple<char, char> tpl(10, 8);
        return &boost::hana::at_c<0>(tpl) + 1 == &boost::hana::at_c<1>(tpl);
    }

    static constexpr bool tuple_directly_iterable()
    {
        // sizeof(buffer_type) == (sizeof(pattern_segment<SegmentLength>) + ...)
        auto dist = sizeof(char) + sizeof(int) + sizeof(double);
        boost::hana::tuple<char, int, double> tpl(3, 9006, 1.23456);
        return std::distance(&boost::hana::at_c<0>(tpl), &boost::hana::at_c<3>(tpl)) == dist;
    }

    // non-std tuple because std's one store elements back-to-front
    using buffer_type = boost::hana::tuple<pattern_segment<SegmentLength>...>;
    buffer_type buffer_;

  public:
    constexpr pattern(auto... info)
        : buffer_(info...)
    {
        static_assert(tuple_store_elements_as_is());
        static_assert(tuple_directly_iterable());
    }

    iterator begin() const override
    {
        return &boost::hana::at_c<0>(buffer_);
    }

    iterator end() const override
    {
        return ++&boost::hana::at_c<sizeof...(SegmentLength) - 1>(buffer_);
        // return reinterpret_cast<segment>(std::next(&buffer_));
    }
};

template <impl::pattern_string Pattern>
constexpr auto make_pattern()
{
    // ReSharper disable once CppInconsistentNaming
#define make_hint() impl::dynamic_pattern(Pattern.buff, std::size(Pattern.buff))

    constexpr auto convert_segments = []<size_t... I>(std::index_sequence<I...>) {
        auto hint = make_hint();
        return pattern<make_hint()[I].length()...>(hint[I].extract()...);
    };

    return convert_segments(std::make_index_sequence<make_hint().length()>());

#undef make_hint
}

template <impl::pattern_string Pattern>
constexpr auto operator""_pat()
{
    return make_pattern<Pattern>();
}
} // namespace fd

namespace std
{
inline size_t size(fd::basic_pattern_segment const &i)
{
    return std::distance(i.begin(), i.end());
}
}